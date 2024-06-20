#include "hal/interfaces/SerialCommunication.hpp"

namespace hal
{
    BufferedSerialCommunicationOnUnbuffered::BufferedSerialCommunicationOnUnbuffered(infra::AtomicByteDeque& buffer, SerialCommunication& delegate)
        : buffer(buffer)
        , delegate(delegate)
    {
        delegate.ReceiveData([this](infra::ConstByteRange data)
            {
                this->buffer.Push(data);
                scheduler.Schedule([this]()
                    {
                        if (HasObserver())
                            GetObserver().DataReceived();
                    });
            });
    }

    BufferedSerialCommunicationOnUnbuffered::~BufferedSerialCommunicationOnUnbuffered()
    {
        delegate.ReceiveData([](infra::ConstByteRange) {});
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
}
