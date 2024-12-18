#ifndef SERVICES_TERMINAL_HPP
#define SERVICES_TERMINAL_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/event/QueueForOneReaderOneIrqWriter.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/Observer.hpp"
#include "services/tracer/Tracer.hpp"
#include <cstddef>

namespace services
{
    class Terminal
    {
    public:
        template<std::size_t MaxQueueSize = 32, std::size_t MaxBuffer = 256, std::size_t MaxHistory = 4>
        using WithMaxQueueAndMaxBufferAndMaxHistory = infra::WithStorage<infra::WithStorage<infra::WithStorage<Terminal, std::array<uint8_t, MaxQueueSize + 1>>, infra::BoundedString::WithStorage<MaxBuffer>>, infra::BoundedDeque<infra::BoundedString>::WithMaxSize<MaxHistory>>;

        explicit Terminal(infra::MemoryRange<uint8_t> bufferQueue, infra::BoundedString& internalBuffer, infra::BoundedDeque<infra::BoundedString>& history, hal::SerialCommunication& communication, services::Tracer& tracer);

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
        infra::BoundedString& buffer;
        infra::BoundedDeque<infra::BoundedString>& history;
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

    class TerminalWithCommandsImpl
        : public TerminalWithCommands
        , public Terminal
    {
    public:
        template<std::size_t MaxQueueSize = 32, std::size_t MaxBuffer = 256, std::size_t MaxHistory = 4>
        using WithMaxQueueAndMaxBufferAndMaxHistory = infra::WithStorage<infra::WithStorage<infra::WithStorage<TerminalWithCommandsImpl, std::array<uint8_t, MaxQueueSize + 1>>, infra::BoundedString::WithStorage<MaxBuffer>>, infra::BoundedDeque<infra::BoundedString>::WithMaxSize<MaxHistory>>;

        TerminalWithCommandsImpl(infra::MemoryRange<uint8_t> bufferQueue, infra::BoundedString& internalBuffer, infra::BoundedDeque<infra::BoundedString>& history, hal::SerialCommunication& communication, services::Tracer& tracer);

    private:
        void OnData(infra::BoundedConstString data) override;
    };
}

#endif
