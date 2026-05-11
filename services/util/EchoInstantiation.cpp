#include "services/util/EchoInstantiation.hpp"

namespace main_
{
    EchoOnSesame::EchoOnSesame(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, infra::BoundedDeque<uint8_t>& windowedReceivedMessage, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory)
        : sesame(cobsSendStorage, cobsReceivedMessage, windowedReceivedMessage, serialCommunication)
        , echo(sesame.windowed, serializerFactory)
    {}

    void EchoOnSesame::Reset()
    {
        echo.Reset();
    }

    void EchoOnSesame::Stop(const infra::Function<void()>& onDone)
    {
        sesame.Stop(onDone);
    }
}
