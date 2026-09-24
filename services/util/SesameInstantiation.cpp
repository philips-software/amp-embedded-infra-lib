#include "services/util/SesameInstantiation.hpp"

namespace main_
{
    Sesame::Sesame(CobsStorageBase& storage, hal::BufferedSerialCommunication& serialCommunication)
        : cobs(storage.cobsSendStorage, storage.cobsReceivedMessage, serialCommunication)
        , windowed(storage.windowedRedReceivedMessage, storage.windowedBlueReceivedMessage, storage.windowedReceiveBuffers, cobs)
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

    Sesame::CobsStorageBase::CobsStorageBase(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage,
        infra::BoundedDeque<uint8_t>& windowedRedReceivedMessage, infra::BoundedDeque<uint8_t>& windowedBlueReceivedMessage, uint8_t windowedReceiveBuffers)
        : cobsSendStorage(cobsSendStorage)
        , cobsReceivedMessage(cobsReceivedMessage)
        , windowedRedReceivedMessage(windowedRedReceivedMessage)
        , windowedBlueReceivedMessage(windowedBlueReceivedMessage)
        , windowedReceiveBuffers(windowedReceiveBuffers)
    {}
}
