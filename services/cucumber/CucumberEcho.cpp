#include "services/cucumber/CucumberEcho.hpp"

namespace services {

    CucumberEchoProto::CucumberEchoProto()
        : proxy(*this)
    {}

    CucumberEchoClient::CucumberEchoClient(AllocatorCucumberEchoProto& allocator, services::ConnectionFactory& connectionFactory, services::IPAddress hostname, services::Tracer& tracer)
        : connectionFactory(connectionFactory)
        , allocator(allocator)
        , hostname(hostname)
        , tracer(tracer)
    {
        tracer.Trace() << "Connecting to " << hostname << "\n";
        connectionFactory.Connect(*this);
    }

    CucumberEchoClient::~CucumberEchoClient()
    {}

    infra::BoundedConstString CucumberEchoClient::Hostname() const
    {
        return "192.168.1.41";
    }

    services::IPAddress CucumberEchoClient::Address() const
    {
        return hostname;
    }

    uint16_t CucumberEchoClient::Port() const
    {
        return 1234;
    }

    void CucumberEchoClient::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        tracer.Trace() << "Connection established to " << hostname << "\n";

        echoProto = allocator.Allocate();
        createdObserver(echoProto);
        services::CucumberContext::Instance().Add("EchoProto", &echoProto);
    }

    void CucumberEchoClient::ConnectionFailed(ConnectFailReason reason)
    {
        tracer.Trace() << "Connection Failed";
    }

    void CucumberEchoClient::LedSet(cucumber_interaction::LedRequest::Led led, bool high)
    {
        echoProto->proxy.RequestSend([=]
        {
            echoProto->proxy.LedSet(cucumber_interaction::LedRequest::Led::Blue, true);
        });
    }



}