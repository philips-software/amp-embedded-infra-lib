#include "services/util/SesameInstantiation.hpp"

namespace main_
{
    Sesame::Sesame(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedDeque<uint8_t>& windowedReceivedMessage,
        hal::BufferedSerialCommunication& serialCommunication)
        : cobs(cobsSendStorage, cobsReceivedMessage, serialCommunication)
        , windowed(windowedReceivedMessage, cobs)
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
}
