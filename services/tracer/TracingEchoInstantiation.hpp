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
    {
        template<std::size_t MessageSize>
        struct WithMessageSize;

        TracingEchoOnSesame(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, services::Tracer& tracer);
        ~TracingEchoOnSesame();

        void Reset();

        services::SesameCobs cobs;
        services::SesameWindowed windowed{ cobs };
        services::TracingEchoOnSesame echo;
    };

    template<std::size_t MessageSize>
    struct TracingEchoOnSesame::WithMessageSize
        : TracingEchoOnSesame
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory, services::Tracer& tracer)
            : TracingEchoOnSesame(cobsSendStorage, cobsReceivedMessage, serialCommunication, serializerFactory, tracer)
        {}

    private:
        static constexpr std::size_t encodedMessageSize = services::SesameWindowed::bufferSizeForMessage<MessageSize, services::SesameCobs::EncodedMessageSize>;

        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameCobs::sendBufferSize<MessageSize>> cobsSendStorage;
        infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameCobs::receiveBufferSize<encodedMessageSize>> cobsReceivedMessage;
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
