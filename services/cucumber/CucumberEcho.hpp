#ifndef SERVICES_CUCUMBER_ECHO_HPP 
#define SERVICES_CUCUMBER_ECHO_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/event/QueueForOneReaderOneIrqWriter.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "services/network/SingleConnectionListener.hpp"
#include "infra/util/BoundedVector.hpp"
#include "services/tracer/Tracer.hpp"
#include "infra/util/SharedObjectAllocatorFixedSize.hpp"
#include "generated/echo/CucumberInteraction.pb.hpp"
#include "services/cucumber/CucumberContext.hpp"
#include "infra/timer/Timer.hpp"

namespace services {

    class CucumberEchoProto
        : public services::EchoOnConnection
    {
    public:
        CucumberEchoProto();

    public:
        cucumber_interaction::CucumberInteractionProxy proxy;
    };

    using AllocatorCucumberEchoProto = infra::SharedObjectAllocator<CucumberEchoProto, void()>;

    class CucumberEchoClient
        : public services::ClientConnectionObserverFactory
    {
    public:
        template<std::size_t MaxConnections>
        using WithMaxConnections = infra::WithStorage<CucumberEchoClient, AllocatorCucumberEchoProto::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>;

        CucumberEchoClient(AllocatorCucumberEchoProto& allocator, services::ConnectionFactory& connectionFactory, services::IPAddress hostname, services::Tracer& tracer);
        ~CucumberEchoClient();

        void LedSet(cucumber_interaction::LedRequest::Led led, bool high);

    public:
        virtual services::IPAddress Address() const;
        virtual infra::BoundedConstString Hostname() const;
        virtual uint16_t Port() const override;
    public:
        void Connect();
        void SetAddress(services::IPAddress address);
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;

    public:
        infra::SharedPtr<CucumberEchoProto> echoProto;

    private:
        services::ConnectionFactory& connectionFactory;
        services::IPAddress hostname;
        services::Tracer& tracer;
        AllocatorCucumberEchoProto& allocator;
    };

    void SetGlobalCucumberEchoClient(services::CucumberEchoClient& echoClient);
    services::CucumberEchoClient& GlobalCucumberEchoClient();
}

#endif