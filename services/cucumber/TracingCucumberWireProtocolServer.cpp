#include "services/cucumber/TracingCucumberWireProtocolServer.hpp"

namespace services
{
    void InitCucumberWireServer(services::ConnectionFactory& connectionFactory, uint16_t port, services::Tracer& tracer)
    {
        static services::CucumberContext context;
        static services::TracingCucumberWireProtocolServer::WithBuffer<512> tracingCucumberServer(connectionFactory, port, tracer);
    }

	TracingCucumberWireProtocolConnectionObserver::TracingCucumberWireProtocolConnectionObserver(const infra::ByteRange receiveBuffer, services::Tracer& tracer)
		: CucumberWireProtocolConnectionObserver::CucumberWireProtocolConnectionObserver(receiveBuffer)
		, tracer(tracer)
		, receiveBuffer(receiveBuffer)
    {
	}

    void TracingCucumberWireProtocolConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        auto tracingWriterPtr = tracingWriter.Emplace(std::move(writer), tracer);
        CucumberWireProtocolConnectionObserver::SendStreamAvailable(infra::MakeContainedSharedObject(tracingWriterPtr->Writer(), std::move(tracingWriterPtr)));
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

    TracingCucumberWireProtocolConnectionObserver::TracingWriter::TracingWriter(infra::SharedPtr<infra::StreamWriter>&& writer, services::Tracer& tracer)
        : writer(std::move(writer))
        , tracingWriter(*this->writer, tracer)
    {}

    infra::StreamWriter& TracingCucumberWireProtocolConnectionObserver::TracingWriter::Writer()
    {
        return tracingWriter;
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
