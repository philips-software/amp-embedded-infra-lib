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
        EchoForwarder(services::Echo& echo, hal::SerialCommunication& serialCommunication, uint32_t serviceIdPeerNode, uint32_t responseIdPeerNode)
            : echoStack(serialCommunication)
            , servicePeerNode(echo, serviceIdPeerNode, echoStack)
            , responsePeerNode(echoStack, responseIdPeerNode, echo)
        {}

        EchoForwarder(services::Echo& echo, hal::SerialCommunication& serialCommunication, uint32_t serviceIdPeerNode, uint32_t responseIdPeerNode
            , uint32_t serviceIdDiCommClient, uint32_t responseIdDiCommClient)
            : echoStack(serialCommunication)
            , servicePeerNode(echo, serviceIdPeerNode, echoStack)
            , responsePeerNode(echoStack, responseIdPeerNode, echo)
        {
            this->serviceDiCommClient.Emplace(echo, serviceIdDiCommClient, echoStack);
            this->responseDiCommClient.Emplace(echoStack, responseIdDiCommClient, echo);
        }

        EchoOnSerialCommunication<MessageSize> echoStack;
        services::ServiceForwarder::WithMaxMessageSize<MessageSize> servicePeerNode;
        services::ServiceForwarder::WithMaxMessageSize<MessageSize> responsePeerNode;
        infra::Optional<services::ServiceForwarder::WithMaxMessageSize<MessageSize>> serviceDiCommClient;
        infra::Optional<services::ServiceForwarder::WithMaxMessageSize<MessageSize>> responseDiCommClient;
    };
}

#endif
