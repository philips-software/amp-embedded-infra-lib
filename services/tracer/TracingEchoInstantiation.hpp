#ifndef SERVICES_TRACING_ECHO_INSTANTIATION_HPP
#define SERVICES_TRACING_ECHO_INSTANTIATION_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/util/EchoInstantiation.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameWindowed.hpp"
#include "services/util/TracingEchoOnSesame.hpp"

namespace main_
{
    struct TracingEchoOnSesame
        : public services::Stoppable
    {
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        TracingEchoOnSesame(Sesame::CobsStorageBase& storage, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, services::Tracer& tracer);

        void Reset();

        // Implementation of Stoppable
        void Stop(const infra::Function<void()>& onDone) override;

        services::SesameCobs cobs;
        services::SesameWindowed windowed;
        services::TracingEchoOnSesame echo;

        infra::AutoResetFunction<void()> onStopDone;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct TracingEchoOnSesame::WithMessageSize
        : private Sesame::CobsStorage<MessageSize, SplitBuffers>
        , TracingEchoOnSesame
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, services::Tracer& tracer)
            : TracingEchoOnSesame(static_cast<CobsStorage<MessageSize, SplitBuffers>&>(*this), serialCommunication, serializerFactory, tracer)
        {}
    };

#ifdef EMIL_HAL_GENERIC
    template<std::size_t MessageSize>
    struct TracingEchoOnUart
    {
        TracingEchoOnUart(infra::BoundedConstString portName, services::Tracer& tracer, const hal::UartGeneric::Config& config = {})
            : uart(infra::AsStdString(portName), config)
            , echoOnSesame(this->bufferedSerial, this->serializerFactory, tracer)
        {}

        hal::UartGeneric uart;
        services::MethodSerializerFactory::OnHeap serializerFactory;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<MessageSize> bufferedSerial{ uart };
        main_::TracingEchoOnSesame::WithMessageSize<MessageSize> echoOnSesame;

        services::Echo& echo{ echoOnSesame.echo };
    };
#endif
}

#endif
