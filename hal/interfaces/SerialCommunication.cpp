#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/stream/AtomicByteQueue.hpp"
#include "infra/stream/InputStream.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"

namespace hal
{
    BufferedSerialCommunicationOnUnbuffered::BufferedSerialCommunicationOnUnbuffered(infra::AtomicByteQueue& buffer, SerialCommunication& delegate)
        : buffer{ buffer }
        , delegate{ delegate }
        , handleDataReceived{ [this]()
            {
                if (HasObserver())
                    GetObserver().DataReceived();
            } }
    {
        delegate.ReceiveData([this](infra::ConstByteRange data)
            {
                this->buffer.Push(data);
                scheduler.Schedule([this]()
                    {
                        handleDataReceived();
                    });
            });
    }

    BufferedSerialCommunicationOnUnbuffered::~BufferedSerialCommunicationOnUnbuffered()
    {
        Stop();
    }

    void BufferedSerialCommunicationOnUnbuffered::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        delegate.SendData(data, actionOnCompletion);
    }

    infra::StreamReaderWithRewinding& BufferedSerialCommunicationOnUnbuffered::Reader()
    {
        return reader;
    }

    void BufferedSerialCommunicationOnUnbuffered::AckReceived()
    {
        buffer.Pop(reader.ConstructSaveMarker());
        reader.Rewind(0);
    }

    void BufferedSerialCommunicationOnUnbuffered::Stop(const infra::Function<void()>& onDone)
    {
        Stop();

        infra::EventDispatcher::Instance().Schedule(onDone);
    }

    void BufferedSerialCommunicationOnUnbuffered::Stop()
    {
        handleDataReceived = infra::emptyFunction;
        delegate.ReceiveData(nullptr);
    }
}
