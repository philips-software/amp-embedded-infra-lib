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
        EchoForwarder(services::Echo& echo, hal::SerialCommunication& serialCommunication, uint32_t toSerialId, uint32_t fromSerialId)
            : echoStack(serialCommunication)
            , toSerial(echo, toSerialId, echoStack.Echo())
            , fromSerial(echoStack.Echo(), fromSerialId, echo)
        {}

        EchoOnSerialCommunication<MessageSize> echoStack;
        services::ServiceForwarder::WithMaxMessageSize<MessageSize> toSerial;
        services::ServiceForwarder::WithMaxMessageSize<MessageSize> fromSerial;
    };
}

#endif
