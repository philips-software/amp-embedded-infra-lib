#include "services/tracer/TracingEchoInstantiation.hpp"

namespace main_
{
    TracingEchoOnSesame::TracingEchoOnSesame(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, services::Tracer& tracer)
        : cobs(cobsSendStorage, cobsReceivedMessage, serialCommunication)
        , echo(serializerFactory, services::echoErrorPolicyAbort, tracer, windowed)
    {}

    void TracingEchoOnSesame::Reset()
    {
        echo.Reset();
    }

    void TracingEchoOnSesame::Stop(const infra::Function<void()>& onDone)
    {
        this->onStopDone = onDone;
        cobs.Stop([this]()
            {
                windowed.Stop();
                this->onStopDone();
            });
    }
}
