#ifndef CUCUMBER_WIRE_PROTOCOL_SERVER_MOCK_HPP
#define CUCUMBER_WIRE_PROTOCOL_SERVER_MOCK_HPP

#include "gmock/gmock.h"
#include "services\cucumber\CucumberWireProtocolServer.hpp"
#include "infra/util/Function.hpp"
#include "services/cucumber/CucumberEcho.hpp"

namespace services
{
    class CucumberWireProtocolServerMock
        : public services::CucumberWireProtocolServer
    {

    };

    class CucumberStepMock
        : public services::CucumberStep
    {
    public:
        CucumberStepMock()
            : CucumberStep("Mock Step")
        {}

        MOCK_METHOD2(Invoke, void(infra::JsonArray& arguments, infra::Function<void(bool)> onDone));
    };

    using AllocatorCucumberEchoProtoMock = infra::SharedObjectAllocator<CucumberEchoProto, void()>;

    class CucumberEchoClientMock
        : public services::CucumberEchoClient
    {
    public:
        template<std::size_t MaxConnections>
        using WithMaxConnections = infra::WithStorage<CucumberEchoClientMock, AllocatorCucumberEchoProtoMock::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>;

        CucumberEchoClientMock(AllocatorCucumberEchoProto& allocator, services::ConnectionFactory& connectionFactory, services::IPAddress hostname, services::Tracer& tracer)
            : connectionFactory(connectionFactory)
            , allocator(allocator)
            , hostname(hostname)
            , tracer(tracer)
            , CucumberEchoClient(allocator, connectionFactory, hostname, tracer)
        {
            ConnectionEstablished([](infra::SharedPtr<services::ConnectionObserver>) {});
        }

        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;

    private:
        services::ConnectionFactory& connectionFactory;
        services::IPAddress hostname;
        services::Tracer& tracer;
        AllocatorCucumberEchoProto& allocator;
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