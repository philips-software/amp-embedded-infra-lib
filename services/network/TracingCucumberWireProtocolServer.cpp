#include "services/network/TracingCucumberWireProtocolServer.hpp"

namespace services
{
	TracingCucumberWireProtocolConnectionObserver::TracingCucumberWireProtocolConnectionObserver(const infra::ByteRange receiveBuffer, services::Tracer& tracer)
		: CucumberWireProtocolConnectionObserver::CucumberWireProtocolConnectionObserver(receiveBuffer)
		, tracer(tracer)
		, receiveBuffer(receiveBuffer)
    {
	}

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

    TracingCucumberWireProtocolServer::TracingCucumberWireProtocolServer(const infra::ByteRange receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port, services::Tracer& tracer)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
		, receiveBuffer(receiveBuffer)
        , tracer(tracer)
        , connectionCreator([this](infra::Optional<TracingCucumberWireProtocolConnectionObserver>& value, services::IPAddress address) {
            this->tracer.Trace() << "CucumberWireProtocolServer connection accepted from: " << address;
            value.Emplace(this->receiveBuffer, this->tracer);
        })
    {}
}
