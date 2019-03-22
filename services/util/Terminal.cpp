#include "services/util/Terminal.hpp"

namespace services
{
    Terminal::Terminal(hal::SerialCommunication& communication)
        : communication(communication)
        , queue([this] { HandleInput(); })
    {
        communication.ReceiveData([this](infra::ConstByteRange data) { for (uint8_t element : data) queue.AddFromInterrupt(element); });
        Print(state.prompt);
    }

    void Terminal::Print(const char* message)
    {
        if (sendDone)
        {
            sendDone = false;
            infra::ConstByteRange data = infra::ReinterpretCastByteRange(infra::MakeRange(message, message + strlen(message)));
            communication.SendData(data, [this]() { sendDone = true; });
        }
    }

    void Terminal::HandleInput()
    {
        while (!queue.Empty())
            HandleChar(static_cast<char>(queue.Get()));
    }

    void Terminal::HandleChar(char c)
    {
        if (state.processingEscapeSequence)
            state.processingEscapeSequence = ProcessEscapeSequence(c);
        else
            HandleNonEscapeChar(c);
    }

    void Terminal::HandleNonEscapeChar(char c)
    {
        switch (c)
        {
            case '\n':
                break;
            case '\r':
                ProcessEnter();
                break;
            case 27:
                state.processingEscapeSequence = true;
                break;
            case '\b':
            case '\x7F':
                ProcessBackspace();
                break;
            case 1: // ctrl-a
                MoveCursorHome();
                break;
            case 2: // ctrl-b
                MoveCursorLeft();
                break;
            case 3: // ctrl-c
                OverwriteBuffer("");
                break;
            case 4: // ctrl-d
                ProcessDelete();
                break;
            case 5: // ctrl-e
                MoveCursorEnd();
                break;
            case 6: // ctrl-f
                MoveCursorRight();
                break;
            case 14: // ctrl-n
                HistoryForward();
                break;
            case 16: // ctrl-p
                HistoryBackward();
                break;
            default:
                SendNonEscapeChar(c);
                break;
        }
    }

    bool Terminal::ProcessEscapeSequence(char in)
    {
        static const infra::BoundedConstString ignoredEscapeCharacters = ";[O0123456789";
        if (ignoredEscapeCharacters.find(in) != infra::BoundedConstString::npos)
            return true;

        switch (in)
        {
            case 'A':
                HistoryBackward();
                break;
            case 'B':
                HistoryForward();
                break;
            case 'C':
                MoveCursorRight();
                break;
            case 'D':
                MoveCursorLeft();
                break;
            case 'F':
                MoveCursorEnd();
                break;
            case 'H':
                MoveCursorHome();
                break;
            default:
                SendBell();
                break;
        }

        return false;
    }

    void Terminal::ProcessEnter()
    {
        Print("\r\n");

        if (buffer.size() > 0)
        {
            StoreHistory(buffer);
            OnData(buffer);
        }

        buffer.clear();
        state.cursorPosition = 0;
        Print(state.prompt);
    }

    void Terminal::ProcessBackspace()
    {
        MoveCursorLeft();
        ProcessDelete();
    }

    void Terminal::ProcessDelete()
    {
        if (state.cursorPosition < buffer.size())
            EraseCharacterUnderCursor();
        else
            SendBell();
    }

    void Terminal::EraseCharacterUnderCursor()
    {
        assert(state.cursorPosition < buffer.size());

        if (buffer.size() == state.cursorPosition + 1)
            buffer.pop_back();
        else
        {
            std::rotate(std::next(buffer.begin(), state.cursorPosition), std::next(buffer.begin(), state.cursorPosition + 1), buffer.end());
            buffer = buffer.substr(0, buffer.size() - 1);

            communication.SendData(infra::MakeRange(reinterpret_cast<const uint8_t*>(std::next(buffer.begin(), state.cursorPosition)), reinterpret_cast<const uint8_t*>(buffer.end())), infra::emptyFunction);
        }

        Print(" \b");

        for (uint32_t i = buffer.size(); i > state.cursorPosition; --i)
            communication.SendData(infra::MakeByteRange('\b'), infra::emptyFunction);
    }

    void Terminal::MoveCursorHome()
    {
        state.cursorPosition = 0;
        communication.SendData(infra::MakeByteRange('\r'), infra::emptyFunction);
        Print(state.prompt);
    }

    void Terminal::MoveCursorEnd()
    {
        if (buffer.size() > 0 && state.cursorPosition < buffer.size())
            communication.SendData(infra::MakeRange(reinterpret_cast<const uint8_t*>(std::next(buffer.begin(), state.cursorPosition)), reinterpret_cast<const uint8_t*>(buffer.end())), infra::emptyFunction);
        state.cursorPosition = buffer.size();
    }

    void Terminal::MoveCursorLeft()
    {
        if (state.cursorPosition > 0)
        {
            communication.SendData(infra::MakeByteRange('\b'), infra::emptyFunction);
            --state.cursorPosition;
        }
        else
            SendBell();
    }

    void Terminal::MoveCursorRight()
    {
        if (state.cursorPosition < buffer.size())
        {
            communication.SendData(infra::MakeByteRange(buffer[state.cursorPosition]), infra::emptyFunction);
            ++state.cursorPosition;
        }
        else
            SendBell();
    }

    void Terminal::StoreHistory(infra::BoundedString element)
    {
        if (history.full())
            history.pop_front();

        history.push_back(buffer);
        state.historyIndex = history.size();
    }

    void Terminal::OverwriteBuffer(infra::BoundedConstString element)
    {
        std::size_t previousSize = buffer.size();
        buffer = element;

        communication.SendData(infra::MakeByteRange('\r'), infra::emptyFunction);
        Print(state.prompt);

        if (buffer.size() > 0)
            communication.SendData(infra::MakeRange(reinterpret_cast<const uint8_t*>(buffer.begin()), reinterpret_cast<const uint8_t*>(buffer.end())), infra::emptyFunction);

        for (std::size_t size = buffer.size(); size < previousSize; ++size)
            communication.SendData(infra::MakeByteRange(' '), infra::emptyFunction);

        for (std::size_t size = buffer.size(); size < previousSize; ++size)
            communication.SendData(infra::MakeByteRange('\b'), infra::emptyFunction);

        state.cursorPosition = buffer.size();
    }

    void Terminal::HistoryForward()
    {
        if (!history.empty() && state.historyIndex < history.size() - 1)
            OverwriteBuffer(history[++state.historyIndex]);
        else
            OverwriteBuffer("");
    }

    void Terminal::HistoryBackward()
    {
        if (state.historyIndex > 0)
            OverwriteBuffer(history[--state.historyIndex]);
        else
            SendBell();
    }

    void Terminal::SendNonEscapeChar(char c)
    {
        if (c > 31 && c < 127)
        {
            communication.SendData(infra::MakeByteRange(c), infra::emptyFunction);
            buffer.push_back(c);
            state.cursorPosition++;
        }
        else
            SendBell();
    }

    void Terminal::SendBell()
    {
        communication.SendData(infra::MakeByteRange('\a'), infra::emptyFunction);
    }
}
