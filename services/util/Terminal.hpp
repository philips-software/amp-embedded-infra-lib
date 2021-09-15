#ifndef SERVICES_TERMINAL_HPP
#define SERVICES_TERMINAL_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/event/QueueForOneReaderOneIrqWriter.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/BoundedString.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class Terminal
    {
    public:
        explicit Terminal(hal::SerialCommunication& communication, services::Tracer& tracer);

        void Print(const char* message);
        virtual void OnData(infra::BoundedString data) {}

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
        TerminalState state;
        hal::SerialCommunication& communication;
        services::Tracer& tracer;
        infra::QueueForOneReaderOneIrqWriter<uint8_t>::WithStorage<32> queue;
        infra::BoundedString::WithStorage<256> buffer;
        infra::BoundedDeque<decltype(buffer)>::WithMaxSize<4> history;
    };
}

#endif
