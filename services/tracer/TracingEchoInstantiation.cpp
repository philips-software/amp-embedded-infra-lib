#include "services/tracer/TracingEchoInstantiation.hpp"

namespace main_
{
    TracingEchoOnSesame::TracingEchoOnSesame(Sesame::CobsStorageBase& storage, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, services::Tracer& tracer)
        : cobs(storage.cobsSendStorage, storage.cobsReceivedMessage, serialCommunication)
        , windowed(storage.windowedReceivedMessage, storage.windowedReceiveBuffers, cobs)
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
                windowed.ResetReading();
                this->onStopDone();
            });
    }
}
