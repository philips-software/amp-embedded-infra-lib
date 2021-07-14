#ifndef PROTOBUF_ECHO_INSTANTIATIONS
#define PROTOBUF_ECHO_INSTANTIATIONS

#include "protobuf/echo/Echo.hpp"
#include "infra/util/BoundedVector.hpp"
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

    template<std::size_t MessageSize, std::size_t MaxServices>
    class EchoForwarder
    {
    public:
        EchoForwarder(services::Echo& echo, hal::SerialCommunication& serialCommunication)
            : echo(echo)
            , echoStack(serialCommunication)
        {}

        void AddService(uint32_t serviceId, uint32_t responseId)
        {
            std::array<uint8_t, MessageSize> storageService;
            std::array<uint8_t, MessageSize> storageResponse;
            vector.emplace_back(storageService, echo, serviceId, echoStack);
            vector.emplace_back(storageResponse, echoStack, responseId, echo);
        }

    private:
        services::Echo& echo;
        EchoOnSerialCommunication<MessageSize> echoStack;

        infra::BoundedVector<services::ServiceForwarder>::WithMaxSize<MaxServices> vector;
    };
}

#endif
