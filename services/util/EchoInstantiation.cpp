#include "services/util/EchoInstantiation.hpp"

namespace main_
{
    EchoOnSesame::EchoOnSesame(Sesame::CobsStorageBase& storage, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory)
        : sesame(storage, serialCommunication)
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
