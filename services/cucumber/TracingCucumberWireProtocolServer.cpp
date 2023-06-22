#include "services/cucumber/TracingCucumberWireProtocolServer.hpp"
#include "services/cucumber/CucumberWireProtocolServer.hpp"

namespace services
{
    TracingCucumberWireProtocolConnectionObserver::TracingCucumberWireProtocolConnectionObserver(CucumberScenarioRequestHandler& scenarioRequestHandler, services::Tracer& tracer)
        : CucumberWireProtocolConnectionObserver::CucumberWireProtocolConnectionObserver(scenarioRequestHandler)
        , tracer(tracer)
    {}

    void TracingCucumberWireProtocolConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        tracer.Trace() << "<-- ";
        auto tracingWriterPtr = tracingWriter.Emplace(std::move(writer), tracer);
        CucumberWireProtocolConnectionObserver::SendStreamAvailable(infra::MakeContainedSharedObject(tracingWriterPtr->Writer(), std::move(tracingWriterPtr)));
    }

    void TracingCucumberWireProtocolConnectionObserver::DataReceived()
    {
        auto reader = ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        tracer.Trace() << "--> ";
        while (!stream.Empty())
            tracer.Continue() << infra::ByteRangeAsString(stream.ContiguousRange());

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

    TracingCucumberWireProtocolServer::TracingCucumberWireProtocolServer(services::ConnectionFactory& connectionFactory, uint16_t port, CucumberScenarioRequestHandler& scenarioRequestHandler, services::Tracer& tracer)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , tracer(tracer)
        , scenarioRequestHandler(scenarioRequestHandler)
        , connectionCreator([this](infra::Optional<TracingCucumberWireProtocolConnectionObserver>& value, services::IPAddress address)
              {
            this->tracer.Trace() << "CucumberWireProtocolServer connection accepted from: " << address;
            value.Emplace(this->scenarioRequestHandler, this->tracer); })
    {
        CucumberWireProtocolServer::InitializeTestDriver();
    }
}
