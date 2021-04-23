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
    {}

    CucumberEchoClient::~CucumberEchoClient()
    {}

    infra::BoundedConstString CucumberEchoClient::Hostname() const
    {
        return "192.168.1.161";
    }

    void CucumberEchoClient::SetAddress(services::IPAddress address)
    {
        hostname = address;
    }

    services::IPAddress CucumberEchoClient::Address() const
    {
        return hostname;
    }

    uint16_t CucumberEchoClient::Port() const
    {
        return 1234;
    }

    void CucumberEchoClient::Connect()
    {
        tracer.Trace() << "Connecting to " << hostname;
        connectionFactory.Connect(*this);
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
        tracer.Trace() << "Connection Failed to EchoClient at : " << hostname;
    }

    void CucumberEchoClient::LedSet(cucumber_interaction::LedRequest::Led led, bool high)
    {
    	if (high)
			echoProto->proxy.RequestSend([=]
				{
					echoProto->proxy.LedSet(led, true);
				});
    	else
    		echoProto->proxy.RequestSend([=]
				{
					echoProto->proxy.LedSet(led, false);
				});
    }

    namespace
    {
        services::CucumberEchoClient* globalCucumberEchoClient = nullptr;
    }

    void SetGlobalCucumberEchoClient(services::CucumberEchoClient& echoClient)
    {
        assert(globalCucumberEchoClient == nullptr);
        globalCucumberEchoClient = &echoClient;
    }

    services::CucumberEchoClient& GlobalCucumberEchoClient()
    {
        assert(globalCucumberEchoClient != nullptr);
        return *globalCucumberEchoClient;
    }

}
