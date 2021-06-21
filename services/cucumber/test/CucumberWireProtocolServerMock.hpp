#ifndef CUCUMBER_WIRE_PROTOCOL_SERVER_MOCK_HPP
#define CUCUMBER_WIRE_PROTOCOL_SERVER_MOCK_HPP

#include "cucumber_test/clients/CucumberEchoClient.hpp"
#include "gmock/gmock.h"
#include "infra/util/Function.hpp"
#include "services\cucumber\CucumberWireProtocolServer.hpp"

namespace services
{

    class CucumberStepMock
        : public services::CucumberStep
    {
    public:
        CucumberStepMock()
            : CucumberStep("Mock Step")
        {}

        MOCK_METHOD1(Invoke, void(infra::JsonArray& arguments));
    };

    using AllocatorCucumberEchoProtoMock = infra::SharedObjectAllocator<application::CucumberEchoProto, void()>;

    class CucumberEchoClientMock
        : public application::CucumberEchoClient
    {
    public:
        template<std::size_t MaxConnections>
        using WithMaxConnections = infra::WithStorage<CucumberEchoClientMock, AllocatorCucumberEchoProtoMock::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>;

        CucumberEchoClientMock(application::AllocatorCucumberEchoProto& allocator, services::ConnectionFactory& connectionFactory, services::IPAddress hostname, services::Tracer& tracer)
            : connectionFactory(connectionFactory)
            , allocator(allocator)
            , hostname(hostname)
            , tracer(tracer)
            , application::CucumberEchoClient(allocator, connectionFactory, hostname, tracer)
        {
            ConnectionEstablished([](infra::SharedPtr<services::ConnectionObserver>) {});
        }

        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;

    private:
        services::ConnectionFactory& connectionFactory;
        services::IPAddress hostname;
        services::Tracer& tracer;
        application::AllocatorCucumberEchoProto& allocator;
    };

    void CucumberEchoClientMock::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        tracer.Trace() << "Connection established to " << hostname << "\n";

        echoProto = allocator.Allocate();
        createdObserver(echoProto);
        services::CucumberContext::Instance().Add("EchoProto", &echoProto);
    }


}

#endif