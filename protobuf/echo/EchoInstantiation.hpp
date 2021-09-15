#ifndef PROTOBUF_ECHO_INSTANTIATIONS
#define PROTOBUF_ECHO_INSTANTIATIONS

#include "infra/util/BoundedVector.hpp"
#include "protobuf/echo/Echo.hpp"
#include "services/util/MessageCommunicationCobs.hpp"
#include "services/util/MessageCommunicationWindowed.hpp"

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
        services::MessageCommunicationWindowed::WithReceiveBuffer<MessageSize> windowed{ cobs };
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
            AddForwarder(echo, serviceId, echoStack);
            AddForwarder(echoStack, responseId, echo);
        }

        void AddResponse(uint32_t responseId)
        {
            AddForwarder(echoStack, responseId, echo);
        }

    private:
        void AddForwarder(services::Echo& forwardFrom, uint32_t id, services::Echo& forwardTo)
        {
            assert(!forwarderStorage.full() && !forwarders.full());
            forwarderStorage.emplace_back();
            forwarders.emplace_back(forwarderStorage.back(), forwardFrom, id, forwardTo);
        }

    private:
        services::Echo& echo;
        EchoOnSerialCommunication<MessageSize> echoStack;

        infra::BoundedVector<services::ServiceForwarder>::WithMaxSize<MaxServices> forwarders;
        typename infra::BoundedVector<std::array<uint8_t, MessageSize>>::template WithMaxSize<MaxServices> forwarderStorage;
    };
}

#endif
