#include "services/network/TracingCucumberWireProtocolServer.hpp"

namespace services
{
    TracingCucumberWireProtocolConnectionObserver::TracingCucumberWireProtocolConnectionObserver(services::Tracer& tracer)
        : tracer(tracer)
    {}

    void TracingCucumberWireProtocolConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        CucumberWireProtocolConnectionObserver::SendStreamAvailable(std::move(writer));
    }

    void TracingCucumberWireProtocolConnectionObserver::DataReceived()
    {
        auto reader = ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        while (!stream.Empty())
            tracer.Trace() << infra::ByteRangeAsString(stream.ContiguousRange());

        reader = nullptr;

        CucumberWireProtocolConnectionObserver::DataReceived();
    }

    TracingCucumberWireProtocolServer::TracingCucumberWireProtocolServer(services::ConnectionFactory& connectionFactory, uint16_t port, services::Tracer& tracer)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , tracer(tracer)
        , connectionCreator([this](infra::Optional<TracingCucumberWireProtocolConnectionObserver>& value, services::IPAddress address) {
            this->tracer.Trace() << "CucumberWireProtocolServer connection accepted from: " << address;
            value.Emplace(this->tracer);
        })
    {}
}
