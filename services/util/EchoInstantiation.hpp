#ifndef SERVICES_UTIL_ECHO_INSTANTIATION_HPP
#define SERVICES_UTIL_ECHO_INSTANTIATION_HPP

#ifdef EMIL_HAL_GENERIC
#include "hal/generic/UartGeneric.hpp"
#endif
#include "infra/util/BoundedVector.hpp"
#include "protobuf/echo/ServiceForwarder.hpp"
#include "services/util/EchoOnMessageCommunication.hpp"
#include "services/util/EchoOnSesame.hpp"
#include "services/util/MessageCommunicationCobs.hpp"
#include "services/util/MessageCommunicationWindowed.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameWindowed.hpp"

namespace main_
{
    template<std::size_t MessageSize>
    struct EchoOnSerialCommunication
    {
        explicit EchoOnSerialCommunication(hal::SerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory)
            : cobs(serialCommunication)
            , echo(windowed, serializerFactory)
        {}

        operator services::Echo&()
        {
            return echo;
        }

        services::MessageCommunicationCobs::WithMaxMessageSize<MessageSize> cobs;
        services::MessageCommunicationWindowed::WithReceiveBuffer<MessageSize> windowed{ cobs };
        services::EchoOnMessageCommunication echo;
    };

    struct EchoOnSesame
    {
        template<std::size_t MessageSize>
        struct WithMessageSize;

        EchoOnSesame(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage, hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory);
        ~EchoOnSesame();

        operator services::Echo&();

        void Reset();

        services::SesameCobs cobs;
        services::SesameWindowed windowed{ cobs };
        services::EchoOnSesame echo;
    };

    template<std::size_t MessageSize>
    struct EchoOnSesame::WithMessageSize
        : EchoOnSesame
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication, services::MethodSerializerFactory& serializerFactory)
            : EchoOnSesame(cobsSendStorage, cobsReceivedMessage, serialCommunication, serializerFactory)
        {}

    private:
        static constexpr std::size_t encodedMessageSize = services::SesameWindowed::bufferSizeForMessage<MessageSize, services::SesameCobs::EncodedMessageSize>;

        infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameCobs::sendBufferSize<MessageSize>> cobsSendStorage;
        infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameCobs::receiveBufferSize<encodedMessageSize>> cobsReceivedMessage;
    };

#ifdef EMIL_HAL_GENERIC
    template<std::size_t MessageSize>
    struct EchoOnUartBase
    {
        explicit EchoOnUartBase(infra::BoundedConstString portName)
            : uart(infra::AsStdString(portName))
        {}

        hal::UartGeneric uart;
        services::MethodSerializerFactory::OnHeap serializerFactory;
        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<MessageSize> bufferedSerial{ uart };
    };

    template<std::size_t MessageSize>
    struct EchoOnUart
        : EchoOnUartBase<MessageSize>
    {
        using EchoOnUartBase<MessageSize>::EchoOnUartBase;

        main_::EchoOnSesame::WithMessageSize<MessageSize> echoOnSesame{ this->bufferedSerial, this->serializerFactory };

        services::Echo& echo{ echoOnSesame.echo };
    };
#endif

    template<std::size_t MessageSize, std::size_t MaxServices>
    class EchoForwarder
    {
    public:
        EchoForwarder(services::Echo& from, services::Echo& to)
            : from(from)
            , to(to)
        {}

        void AddService(uint32_t serviceId, uint32_t responseId)
        {
            AddForwarder(from, serviceId, to);
            AddForwarder(to, responseId, from);
        }

        void AddRequest(uint32_t serviceId)
        {
            AddForwarder(from, serviceId, to);
        }

        void AddResponse(uint32_t responseId)
        {
            AddForwarder(to, responseId, from);
        }

    private:
        void AddForwarder(services::Echo& forwardFrom, uint32_t id, services::Echo& forwardTo)
        {
            forwarders.emplace_back(forwardFrom, id, forwardTo);
        }

    private:
        services::Echo& from;
        services::Echo& to;

        typename infra::BoundedVector<services::ServiceForwarder>::template WithMaxSize<MaxServices> forwarders;
    };

    template<std::size_t MessageSize, std::size_t MaxServices>
    struct EchoForwarderToSerialCommunication
    {
        EchoForwarderToSerialCommunication(services::Echo& from, hal::SerialCommunication& toSerial, services::MethodSerializerFactory& serializerFactory)
            : to(toSerial, serializerFactory)
            , echoForwarder(from, to)
        {}

        EchoOnSerialCommunication<MessageSize> to;
        EchoForwarder<MessageSize, MaxServices> echoForwarder;
    };

    template<std::size_t MessageSize, std::size_t MaxServices>
    struct EchoForwarderToSesame
    {
        EchoForwarderToSesame(services::Echo& from, hal::SerialCommunication& toSerial, services::MethodSerializerFactory& serializerFactory)
            : bufferedSerial(toSerial)
            , to(bufferedSerial, serializerFactory)
            , echoForwarder(from, to)
        {}

        hal::BufferedSerialCommunicationOnUnbuffered::WithStorage<MessageSize> bufferedSerial;
        EchoOnSesame::WithMessageSize<MessageSize> to;
        EchoForwarder<MessageSize, MaxServices> echoForwarder;
    };
}

#endif
