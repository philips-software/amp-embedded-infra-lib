#ifndef SERVICES_TERMINAL_HPP
#define SERVICES_TERMINAL_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/event/QueueForOneReaderOneIrqWriter.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/Tokenizer.hpp"
#include "services/tracer/Tracer.hpp"
#include <cstddef>

namespace services
{
    template<size_t MaxCommandLength = 256>
    class Terminal
    {
    public:
        constexpr static std::size_t MaxBuffer = MaxCommandLength;

        template<std::size_t MaxQueueSize = 32, std::size_t MaxHistory = 4>
        using WithMaxQueueAndMaxHistory = infra::WithStorage<infra::WithStorage<Terminal, std::array<uint8_t, MaxQueueSize + 1>>, typename infra::BoundedDeque<infra::BoundedString::WithStorage<MaxBuffer>>::template WithMaxSize<MaxHistory>>;

        explicit Terminal(infra::MemoryRange<uint8_t> bufferQueue, infra::BoundedDeque<infra::BoundedString::WithStorage<MaxBuffer>>& history, hal::SerialCommunication& communication, services::Tracer& tracer);

        void Print(const char* message);

        virtual void OnData(infra::BoundedConstString data)
        {}

    private:
        void HandleInput();
        void HandleChar(char in);
        void HandleNonEscapeChar(char c);

        bool ProcessEscapeSequence(char in);
        void ProcessEnter();
        void ProcessBackspace();
        void ProcessDelete();

        void EraseCharacterUnderCursor();
        void MoveCursorHome();
        void MoveCursorEnd();
        void MoveCursorLeft();
        void MoveCursorRight();

        void OverwriteBuffer(infra::BoundedConstString element);
        void StoreHistory(infra::BoundedString element);
        void HistoryForward();
        void HistoryBackward();

        void SendNonEscapeChar(char c);
        void SendBell();

    private:
        struct TerminalState
        {
            const char* prompt = "> ";
            bool processingEscapeSequence = false;
            uint32_t cursorPosition = 0;
            uint32_t historyIndex = 0;
        };

    private:
        infra::QueueForOneReaderOneIrqWriter<uint8_t> queue;
        infra::BoundedString::WithStorage<MaxBuffer> buffer;
        infra::BoundedDeque<decltype(buffer)>& history;
        TerminalState state;
        services::Tracer& tracer;
    };

    class TerminalWithCommands;

    class TerminalCommands
        : public infra::Observer<TerminalCommands, TerminalWithCommands>
    {
    public:
        using infra::Observer<TerminalCommands, TerminalWithCommands>::Observer;

        struct CommandInfo
        {
            CommandInfo(infra::BoundedConstString longName, infra::BoundedConstString shortName, infra::BoundedConstString description,
                infra::BoundedConstString params = "")
                : longName(longName)
                , shortName(shortName)
                , description(description)
                , params(params)
                , size(longName.size() + shortName.size() + params.size())
            {}

            infra::BoundedConstString longName;
            infra::BoundedConstString shortName;
            infra::BoundedConstString description;
            infra::BoundedConstString params;
            const std::size_t size;
        };

        struct Command
        {
            const CommandInfo info;
            infra::Function<void(const infra::BoundedConstString& params)> function;
        };

        virtual infra::MemoryRange<const Command> Commands() = 0;

        bool ProcessCommand(infra::BoundedConstString data);
    };

    class TerminalWithCommands
        : public infra::Subject<TerminalCommands>
    {};

    template<size_t MaxCommandLength = 256>
    class TerminalWithCommandsImpl
        : public TerminalWithCommands
        , public Terminal<MaxCommandLength>
    {
    public:
        template<std::size_t MaxQueueSize = 32, std::size_t MaxHistory = 4>
        using WithMaxQueueAndMaxHistory = infra::WithStorage<infra::WithStorage<TerminalWithCommandsImpl, std::array<uint8_t, MaxQueueSize + 1>>, typename infra::BoundedDeque<infra::BoundedString::WithStorage<Terminal<MaxCommandLength>::MaxBuffer>>::template WithMaxSize<MaxHistory>>;

        TerminalWithCommandsImpl(infra::MemoryRange<uint8_t> bufferQueue, infra::BoundedDeque<infra::BoundedString::WithStorage<Terminal<MaxCommandLength>::MaxBuffer>>& history, hal::SerialCommunication& communication, services::Tracer& tracer);

    private:
        void OnData(infra::BoundedConstString data) override;
    };

    template<size_t MaxCommandLength>
    Terminal<MaxCommandLength>::Terminal(infra::MemoryRange<uint8_t> bufferQueue, infra::BoundedDeque<infra::BoundedString::WithStorage<MaxBuffer>>& history, hal::SerialCommunication& communication, services::Tracer& tracer)
        : queue(bufferQueue, [this]
              {
                  HandleInput();
              })
        , history(history)
        , tracer(tracer)
    {
        communication.ReceiveData([this](infra::ConstByteRange data)
            {
                queue.AddFromInterrupt(data);
            });
        Print(state.prompt);
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::Print(const char* message)
    {
        tracer.Continue() << message;
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::HandleInput()
    {
        while (!queue.Empty())
            HandleChar(static_cast<char>(queue.Get()));
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::HandleChar(char c)
    {
        if (state.processingEscapeSequence)
            state.processingEscapeSequence = ProcessEscapeSequence(c);
        else
            HandleNonEscapeChar(c);
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::HandleNonEscapeChar(char c)
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

    template<size_t MaxCommandLength>
    bool Terminal<MaxCommandLength>::ProcessEscapeSequence(char in)
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

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::ProcessEnter()
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

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::ProcessBackspace()
    {
        MoveCursorLeft();
        ProcessDelete();
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::ProcessDelete()
    {
        if (state.cursorPosition < buffer.size())
            EraseCharacterUnderCursor();
        else
            SendBell();
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::EraseCharacterUnderCursor()
    {
        assert(state.cursorPosition < buffer.size());

        if (buffer.size() == state.cursorPosition + 1)
            buffer.pop_back();
        else
        {
            std::rotate(std::next(buffer.begin(), state.cursorPosition), std::next(buffer.begin(), state.cursorPosition + 1), buffer.end());
            buffer = buffer.substr(0, buffer.size() - 1);
            tracer.Continue() << ByteRangeAsString(infra::MakeRange(reinterpret_cast<const uint8_t*>(std::next(buffer.begin(), state.cursorPosition)), reinterpret_cast<const uint8_t*>(buffer.end())));
        }

        Print(" \b");

        for (uint32_t i = buffer.size(); i > state.cursorPosition; --i)
            tracer.Continue() << '\b';
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::MoveCursorHome()
    {
        state.cursorPosition = 0;
        tracer.Continue() << '\r';
        Print(state.prompt);
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::MoveCursorEnd()
    {
        if (buffer.size() > 0 && state.cursorPosition < buffer.size())
            tracer.Continue() << ByteRangeAsString(infra::MakeRange(reinterpret_cast<const uint8_t*>(std::next(buffer.begin(), state.cursorPosition)), reinterpret_cast<const uint8_t*>(buffer.end())));
        state.cursorPosition = buffer.size();
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::MoveCursorLeft()
    {
        if (state.cursorPosition > 0)
        {
            tracer.Continue() << '\b';
            --state.cursorPosition;
        }
        else
            SendBell();
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::MoveCursorRight()
    {
        if (state.cursorPosition < buffer.size())
        {
            tracer.Continue() << buffer[state.cursorPosition];
            ++state.cursorPosition;
        }
        else
            SendBell();
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::StoreHistory(infra::BoundedString element)
    {
        if (history.full())
            history.pop_front();

        history.push_back(buffer);
        state.historyIndex = history.size();
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::OverwriteBuffer(infra::BoundedConstString element)
    {
        std::size_t previousSize = buffer.size();
        buffer.assign(element);

        tracer.Continue() << '\r';
        Print(state.prompt);

        if (buffer.size() > 0)
            tracer.Continue() << buffer;

        for (std::size_t size = buffer.size(); size < previousSize; ++size)
            tracer.Continue() << ' ';

        for (std::size_t size = buffer.size(); size < previousSize; ++size)
            tracer.Continue() << '\b';

        state.cursorPosition = buffer.size();
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::HistoryForward()
    {
        if (!history.empty() && state.historyIndex < history.size() - 1)
            OverwriteBuffer(history[++state.historyIndex]);
        else
            OverwriteBuffer("");
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::HistoryBackward()
    {
        if (state.historyIndex > 0)
            OverwriteBuffer(history[--state.historyIndex]);
        else
            SendBell();
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::SendNonEscapeChar(char c)
    {
        if (c > 31 && c < 127)
        {
            tracer.Continue() << c;
            buffer.push_back(c);
            state.cursorPosition++;
        }
        else
            SendBell();
    }

    template<size_t MaxCommandLength>
    void Terminal<MaxCommandLength>::SendBell()
    {
        tracer.Continue() << '\a';
    }

    bool TerminalCommands::ProcessCommand(infra::BoundedConstString data)
    {
        infra::Tokenizer tokenizer(data, ' ');
        infra::BoundedConstString command = tokenizer.Token(0);
        infra::BoundedConstString params = tokenizer.TokenAndRest(1);

        auto commands = Commands();
        auto it = std::find_if(commands.begin(), commands.end(), [command](const Command& entry)
            {
                return (command == entry.info.longName) || (command == entry.info.shortName);
            });

        if (it != commands.end())
        {
            it->function(params);
            return true;
        }
        else
            return false;
    }

    template<size_t MaxCommandLength>
    TerminalWithCommandsImpl<MaxCommandLength>::TerminalWithCommandsImpl(infra::MemoryRange<uint8_t> bufferQueue, infra::BoundedDeque<infra::BoundedString::WithStorage<Terminal<MaxCommandLength>::MaxBuffer>>& history, hal::SerialCommunication& communication, services::Tracer& tracer)
        : services::Terminal<MaxCommandLength>(bufferQueue, history, communication, tracer)
    {}

    template<size_t MaxCommandLength>
    void TerminalWithCommandsImpl<MaxCommandLength>::OnData(infra::BoundedConstString data)
    {
        bool commandProcessed = NotifyObservers([data](TerminalCommands& observer)
            {
                return observer.ProcessCommand(data);
            });

        if (!commandProcessed)
            Terminal<MaxCommandLength>::Print("Unrecognized command.");
    }
}

#endif
