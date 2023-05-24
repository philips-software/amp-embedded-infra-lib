#ifndef PROTOBUF_ECHO_INSTANTIATIONS
#define PROTOBUF_ECHO_INSTANTIATIONS

#include "infra/util/BoundedVector.hpp"
#include "protobuf/echo/ServiceForwarder.hpp"
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

        operator services::Echo&()
        {
            return echo;
        }

        services::MessageCommunicationCobs::WithMaxMessageSize<MessageSize> cobs;
        services::MessageCommunicationWindowed::WithReceiveBuffer<MessageSize> windowed{ cobs };
        services::EchoOnMessageCommunication echo{ windowed };
    };

    class EchoForwarder
    {
    protected:
        EchoForwarder() = default;
        EchoForwarder(const EchoForwarder& other) = delete;
        EchoForwarder& operator=(const EchoForwarder& other) = delete;
        ~EchoForwarder() = default;

    public:
        virtual void AddService(uint32_t serviceId, uint32_t responseId) = 0;
        virtual void AddResponse(uint32_t responseId) = 0;
    };

    template<std::size_t MessageSize, std::size_t MaxServices>
    class EchoForwarderImpl
        : public EchoForwarder
    {
    public:
        EchoForwarderImpl(services::Echo& from, services::Echo& to)
            : from(from)
            , to(to)
        {}

        virtual void AddService(uint32_t serviceId, uint32_t responseId) override
        {
            AddForwarder(from, serviceId, to);
            AddForwarder(to, responseId, from);
        }

        virtual void AddResponse(uint32_t responseId) override
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

        typename infra::BoundedVector<services::ServiceForwarder::WithMaxMessageSize<MessageSize>>::template WithMaxSize<MaxServices> forwarders;
    };

    template<std::size_t MessageSize, std::size_t MaxServices>
    class EchoForwarderToSerial
        : public EchoForwarder
    {
    public:
        EchoForwarderToSerial(services::Echo& from, hal::SerialCommunication& toSerial)
            : to(toSerial)
            , echoForwarder(from, to)
        {}

        virtual void AddService(uint32_t serviceId, uint32_t responseId) override
        {
            echoForwarder.AddService(serviceId, responseId);
        }

        virtual void AddResponse(uint32_t responseId) override
        {
            echoForwarder.AddResponse(responseId);
        }

    private:
        EchoOnSerialCommunication<MessageSize> to;
        EchoForwarderImpl<MessageSize, MaxServices> echoForwarder;
    };
}

#endif
