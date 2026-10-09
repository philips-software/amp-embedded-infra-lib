#include "services/util/SesameInstantiation.hpp"

namespace main_
{
    Sesame::Sesame(CobsStorageBase& storage, hal::BufferedSerialCommunication& serialCommunication, services::SesameInitializer& initializer)
        : cobs(storage.cobsSendMessage, storage.cobsReceivedMessage, serialCommunication)
        , windowed(storage.windowedReceivedMessage, storage.windowedReceiveBuffers, cobs, initializer)
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

    Sesame::CobsStorageBase::CobsStorageBase(infra::BoundedVector<uint8_t>& cobsSendMessage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage,
        infra::BoundedDeque<uint8_t>& windowedReceivedMessage, uint8_t windowedReceiveBuffers)
        : cobsSendMessage(cobsSendMessage)
        , cobsReceivedMessage(cobsReceivedMessage)
        , windowedReceivedMessage(windowedReceivedMessage)
        , windowedReceiveBuffers(windowedReceiveBuffers)
    {}
}
