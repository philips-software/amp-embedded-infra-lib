#ifndef PROTOBUF_ECHO_INSTANTIATIONS
#define PROTOBUF_ECHO_INSTANTIATIONS

#include "protobuf/echo/Echo.hpp"
#include "services/network/MessageCommunicationCobs.hpp"

namespace main_
{
    template<std::size_t MessageSize>
    struct EchoOnSerialCommunication
    {
        explicit EchoOnSerialCommunication(hal::SerialCommunication& serialCommunication)
            : cobs(serialCommunication)
        {}

        operator services::Echo&() { return echo; }

        services::MessageCommunicationCobs::WithMaxMessageSize<MessageSize> cobs;
        services::WindowedMessageCommunication::WithReceiveBuffer<MessageSize> windowed{ cobs };
        services::EchoOnMessageCommunication echo{ windowed };
    };

    template<std::size_t MessageSize>
    struct EchoForwarder
    {
        EchoForwarder(services::Echo& echoFrom, services::Echo& echoTo, uint32_t serviceId, uint32_t responseId)
            : service(echoFrom, serviceId, echoTo)
            , response(echoTo, responseId, echoFrom)
        {}

        services::ServiceForwarder::WithMaxMessageSize<MessageSize> service;
        services::ServiceForwarder::WithMaxMessageSize<MessageSize> response;
    };
}

#endif
