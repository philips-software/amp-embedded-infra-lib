#include "services/util/EchoInstantiation.hpp"

namespace main_
{
    EchoOnSesame::EchoOnSesame(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory)
        : cobs(cobsSendStorage, cobsReceivedMessage, serialCommunication)
        , echo(windowed, serializerFactory)
    {}

    void EchoOnSesame::Reset()
    {
        echo.Reset();
    }

    void EchoOnSesame::Stop(const infra::Function<void()>& onDone)
    {
        this->onStopDone = onDone;
        cobs.Stop([this]()
            {
                windowed.Stop();
                this->onStopDone();
            });
    }
}
