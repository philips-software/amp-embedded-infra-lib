#include "infra/event/EventDispatcher.hpp"
#include "infra/stream/BoundedDequeOutputStream.hpp"
#include "infra/util/PostAssign.hpp"
#include "services/util/MessageCommunicationWindowed.hpp"

namespace services
{
    MessageCommunicationWindowed::MessageCommunicationWindowed(detail::AtomicDeque& receivedData, MessageCommunicationReceiveOnInterrupt& messageCommunication)
        : MessageCommunicationReceiveOnInterruptObserver(messageCommunication)
        , receivedData(receivedData)
        , state(infra::InPlaceType<StateSendingInit>(), *this)
    {}

    void MessageCommunicationWindowed::RequestSendMessage(uint16_t size)
    {
        state->RequestSendMessage(size);
    }

    std::size_t MessageCommunicationWindowed::MaxSendMessageSize() const
    {
        return MessageCommunicationReceiveOnInterruptObserver::Subject().MaxSendMessageSize() - sizeof(Operation);
    }

    void MessageCommunicationWindowed::ReceivedMessageOnInterrupt(infra::StreamReader& reader)
    {
        infra::DataInputStream::WithErrorPolicy stream(reader, infra::noFail);
        switch (stream.Extract<Operation>())
        {
            case Operation::init:
                sendInitResponse = true;
                otherAvailableWindow = stream.Extract<infra::LittleEndian<uint16_t>>();
                initialized = true;
                break;
            case Operation::initResponse:
                otherAvailableWindow = stream.Extract<infra::LittleEndian<uint16_t>>();
                initialized = true;
                break;
            case Operation::releaseWindow:
                if (initialized)
                    otherAvailableWindow += stream.Extract<infra::LittleEndian<uint16_t>>();
                break;
            case Operation::message:
                if (initialized)
                    ReceivedMessage(reader);
                break;
        }

        infra::EventDispatcher::Instance().Schedule([this]()
        {
            SetNextState();
        });
    }

    void MessageCommunicationWindowed::ReceivedMessage(infra::StreamReader& reader)
    {
        uint16_t size = static_cast<uint16_t>(reader.Available());

        infra::DataInputStream::WithErrorPolicy from(reader, infra::noFail);
        infra::DataOutputStream::WithWriter<detail::AtomicDequeWriter> to(receivedData, infra::noFail);
        assert(size <= to.Available());
        to << size;

        while (size != 0)
        {
            auto range = from.ContiguousRange(size);
            to << range;
            size -= static_cast<uint16_t>(range.size());
        }

        EvaluateReceiveMessage();
    }

    bool MessageCommunicationWindowed::EvaluateReceiveMessage()
    {
        if (!notificationScheduled && reader.Allocatable())
        {
            infra::DataInputStream::WithReader<detail::AtomicDequeReader> stream(receivedData, infra::softFail);
            auto size = stream.Extract<uint16_t>();
            if (!stream.Failed() && stream.Available() >= size)
            {
                if (!notificationScheduled.exchange(true))
                {
                    infra::EventDispatcher::Instance().Schedule([this]()
                    {
                        infra::DataInputStream::WithReader<detail::AtomicDequeReader> stream(receivedData, infra::softFail);
                        auto size = stream.Extract<uint16_t>();
                        if (!stream.Failed() && stream.Available() >= size)
                        {
                            reader.OnAllocatable([this, size]()
                            {
                                receivedData.Pop(size);
                                releasedWindowBuffer += size + 2;
                                if (!EvaluateReceiveMessage())
                                {
                                    releasedWindow += releasedWindowBuffer;
                                    releasedWindowBuffer = 0;
                                    SetNextState();
                                }
                            });
                            receivedData.Pop(2);

                            GetObserver().ReceivedMessage(reader.Emplace(infra::inPlace, receivedData, size));
                        }

                        notificationScheduled = false;
                        EvaluateReceiveMessage();
                    });

                    return true;
                }
            }
        }

        return false;
    }

    void MessageCommunicationWindowed::SetNextState()
    {
        if (!switchingState.exchange(true))
        {
            if (!sending)
            {
                if (sendInitResponse)
                {
                    if (receivedData.Empty())
                        state.Emplace<StateSendingInitResponse>(*this);
                }
                else if (requestedSendMessageSize && WindowSize(*requestedSendMessageSize) <= otherAvailableWindow)
                    state.Emplace<StateSendingMessage>(*this);
                else if (releasedWindow != 0)
                    state.Emplace<StateSendingReleaseWindow>(*this);
                else
                    state.Emplace<StateOperational>(*this);
            }

            switchingState = false;
        }
    }

    uint16_t MessageCommunicationWindowed::WindowSize(uint16_t messageSize) const
    {
        return messageSize + sizeof(uint16_t);
    }

    uint16_t MessageCommunicationWindowed::AvailableWindow() const
    {
        return static_cast<uint16_t>(receivedData.MaxSize() - receivedData.Size());
    }

    MessageCommunicationWindowed::PacketInit::PacketInit(uint16_t window)
        : window(window)
    {}

    MessageCommunicationWindowed::PacketInitResponse::PacketInitResponse(uint16_t window)
        : window(window)
    {}

    MessageCommunicationWindowed::PacketReleaseWindow::PacketReleaseWindow(uint16_t window)
        : window(window)
    {}

    MessageCommunicationWindowed::StateSendingInit::StateSendingInit(MessageCommunicationWindowed& communication)
        : communication(communication)
    {
        communication.sending = true;
        auto writer = communication.MessageCommunicationReceiveOnInterruptObserver::Subject().SendMessageStream(3, [this](uint16_t) { OnSent(); });
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInit(communication.AvailableWindow());

        communication.receivedData.Pop(communication.receivedData.Size());
    }

    void MessageCommunicationWindowed::StateSendingInit::OnSent()
    {
        communication.sending = false;
        communication.SetNextState();
    }

    void MessageCommunicationWindowed::StateSendingInit::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    MessageCommunicationWindowed::StateSendingInitResponse::StateSendingInitResponse(MessageCommunicationWindowed& communication)
        : communication(communication)
    {
        assert(communication.receivedData.Empty());
        communication.sending = true;
        auto writer = communication.MessageCommunicationReceiveOnInterruptObserver::Subject().SendMessageStream(3, [this](uint16_t) { OnSent(); });
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketInitResponse(communication.AvailableWindow());

        communication.releasedWindow = 0;
        communication.sendInitResponse = false;
    }

    void MessageCommunicationWindowed::StateSendingInitResponse::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void MessageCommunicationWindowed::StateSendingInitResponse::OnSent()
    {
        communication.sending = false;
        communication.SetNextState();
    }

    MessageCommunicationWindowed::StateOperational::StateOperational(MessageCommunicationWindowed& communication)
        : communication(communication)
    {}

    void MessageCommunicationWindowed::StateOperational::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
        communication.SetNextState();
    }

    MessageCommunicationWindowed::StateSendingMessage::StateSendingMessage(MessageCommunicationWindowed& communication)
        : communication(communication)
    {
        communication.sending = true;
        auto writer = communication.MessageCommunicationReceiveOnInterruptObserver::Subject().SendMessageStream(*communication.requestedSendMessageSize + 1, [this](uint16_t sent) { OnSent(sent); });

        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << Operation::message;

        communication.requestedSendMessageSize = infra::none;
        communication.GetObserver().SendMessageStreamAvailable(std::move(writer));
    }

    void MessageCommunicationWindowed::StateSendingMessage::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void MessageCommunicationWindowed::StateSendingMessage::OnSent(uint16_t sent)
    {
        communication.otherAvailableWindow -= communication.WindowSize(sent - 1);
        communication.sending = false;
        communication.SetNextState();
    }

    MessageCommunicationWindowed::StateSendingReleaseWindow::StateSendingReleaseWindow(MessageCommunicationWindowed& communication)
        : communication(communication)
    {
        communication.sending = true;
        auto writer = communication.MessageCommunicationReceiveOnInterruptObserver::Subject().SendMessageStream(3, [this](uint16_t sent) { OnSent(); });

        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        stream << PacketReleaseWindow(communication.releasedWindow.exchange(0));
    }

    void MessageCommunicationWindowed::StateSendingReleaseWindow::RequestSendMessage(uint16_t size)
    {
        communication.requestedSendMessageSize = size;
    }

    void MessageCommunicationWindowed::StateSendingReleaseWindow::OnSent()
    {
        communication.sending = false;
        communication.SetNextState();
    }

    namespace detail
    {
        AtomicDeque::AtomicDeque(infra::ByteRange storage)
            : storage(storage)
        {}

        void AtomicDeque::Push(infra::ConstByteRange range)
        {
            assert(range.size() <= MaxSize() - Size());

            while (!range.empty())
            {
                auto to = infra::Head(infra::ByteRange(e, storage.end()), range.size());
                infra::Copy(infra::Head(range, to.size()), to);
                range.pop_front(to.size());
                e += to.size();
                if (e == storage.end())
                    e = storage.begin();
            }

            assert(storage.begin() <= e && e < storage.end());
            assert(storage.begin() <= b && b <= storage.end());
        }

        void AtomicDeque::Pop(std::size_t size)
        {
            auto takenFromEnd = std::min<std::size_t>(size, storage.end() - b);
            b += takenFromEnd;

            assert(storage.begin() <= b && b <= storage.end());
            assert(storage.begin() <= e && e <= storage.end());

            if (b == storage.end())
                b = storage.begin() + size - takenFromEnd;

            assert(storage.begin() <= b && b < storage.end());
            assert(storage.begin() <= e && e <= storage.end());
        }

        std::size_t AtomicDeque::Size() const
        {
            const uint8_t* begin = b;
            const uint8_t* end = e;

            if (begin <= end)
                return end - begin;
            else
                return storage.size() + (end - begin);
        }

        std::size_t AtomicDeque::MaxSize() const
        {
            return storage.size() - 1;
        }

        bool AtomicDeque::Empty() const
        {
            auto end = e.load();
            auto begin = b.load();
            return begin == end;
        }

        infra::ConstByteRange AtomicDeque::PeekContiguousRange(std::size_t start) const
        {
            auto end = e.load();
            auto begin = b.load();

            auto takenFromEnd = std::min<std::size_t>(start, storage.end() - begin);
            begin += takenFromEnd;

            if (begin == storage.end())
                begin = storage.begin() + start - takenFromEnd;

            if (begin <= end)
                return infra::ConstByteRange(begin, end);
            else
                return infra::ConstByteRange(begin, storage.end());
        }

        AtomicDequeReader::AtomicDequeReader(const AtomicDeque& deque)
            : deque(deque)
        {}

        void AtomicDequeReader::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
        {
            auto available = deque.Size();
            while (!range.empty())
            {
                auto dequeRange = infra::Head(PeekContiguousRange(0), range.size());
                errorPolicy.ReportResult(!dequeRange.empty());
                if (dequeRange.empty())
                    break;
                infra::Copy(dequeRange, infra::Head(range, dequeRange.size()));
                range.pop_front(dequeRange.size());
                offset += dequeRange.size();
            }
        }

        uint8_t AtomicDequeReader::Peek(infra::StreamErrorPolicy& errorPolicy)
        {
            auto range = PeekContiguousRange(0);
            errorPolicy.ReportResult(!range.empty());
            if (!range.empty())
                return range.front();
            else
                return 0;
        }

        infra::ConstByteRange AtomicDequeReader::ExtractContiguousRange(std::size_t max)
        {
            auto range = infra::Head(PeekContiguousRange(0), max);
            offset += range.size();
            return range;
        }

        infra::ConstByteRange AtomicDequeReader::PeekContiguousRange(std::size_t start)
        {
            return deque.PeekContiguousRange(start + offset);
        }

        bool AtomicDequeReader::Empty() const
        {
            return deque.Size() == offset;
        }

        std::size_t AtomicDequeReader::Available() const
        {
            return deque.Size() - offset;
        }

        std::size_t AtomicDequeReader::ConstructSaveMarker() const
        {
            return offset;
        }

        void AtomicDequeReader::Rewind(std::size_t marker)
        {
            offset = marker;
        }

        AtomicDequeWriter::AtomicDequeWriter(AtomicDeque& deque)
            : deque(deque)
        {}

        void AtomicDequeWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
        {
            auto available = Available();
            errorPolicy.ReportResult(range.size() <= available);
            deque.Push(infra::Head(range, available));
        }

        std::size_t AtomicDequeWriter::Available() const
        {
            return deque.MaxSize() - deque.Size();
        }

        std::size_t AtomicDequeWriter::ConstructSaveMarker() const
        {
            std::abort();
        }

        std::size_t AtomicDequeWriter::GetProcessedBytesSince(std::size_t marker) const
        {
            std::abort();
        }

        infra::ByteRange AtomicDequeWriter::SaveState(std::size_t marker)
        {
            std::abort();
        }

        void AtomicDequeWriter::RestoreState(infra::ByteRange range)
        {
            std::abort();
        }

        infra::ByteRange AtomicDequeWriter::Overwrite(std::size_t marker)
        {
            std::abort();
        }
    }
}
