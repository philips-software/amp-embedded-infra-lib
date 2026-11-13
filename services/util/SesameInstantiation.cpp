#include "services/util/SesameInstantiation.hpp"

namespace main_
{
    Sesame::Sesame(StorageBase& storage, hal::BufferedSerialCommunication& serialCommunication, services::SesameInitializer& initializer)
        : cobs(storage.cobsSendMessage, storage.cobsReceivedMessage, serialCommunication)
        , windowed(storage.windowedRedReceivedMessage, storage.windowedBlueReceivedMessage, storage.windowedReceiveBuffers, cobs, initializer)
    {}

    void Sesame::Stop(const infra::Function<void()>& onDone)
    {
        this->onStopDone = onDone;
        cobs.Stop([this]()
            {
                windowed.ResetReading();
                this->onStopDone();
            });
    }

    Sesame::StorageBase::StorageBase(infra::BoundedVector<uint8_t>& cobsSendMessage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage,
        infra::BoundedDeque<uint8_t>& windowedRedReceivedMessage, infra::BoundedDeque<uint8_t>& windowedBlueReceivedMessage, uint8_t windowedReceiveBuffers)
        : cobsSendMessage(cobsSendMessage)
        , cobsReceivedMessage(cobsReceivedMessage)
        , windowedRedReceivedMessage(windowedRedReceivedMessage)
        , windowedBlueReceivedMessage(windowedBlueReceivedMessage)
        , windowedReceiveBuffers(windowedReceiveBuffers)
    {}
}
