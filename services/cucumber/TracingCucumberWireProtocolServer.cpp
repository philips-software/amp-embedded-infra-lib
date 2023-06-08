#include "services/cucumber/TracingCucumberWireProtocolServer.hpp"

namespace services
{
    TracingCucumberWireProtocolConnectionObserver::TracingCucumberWireProtocolConnectionObserver(infra::BoundedString& receiveBuffer, CucumberScenarioRequestHandler& scenarioRequestHandler, services::Tracer& tracer)
        : CucumberWireProtocolConnectionObserver::CucumberWireProtocolConnectionObserver(receiveBuffer, scenarioRequestHandler)
        , tracer(tracer)
    {}

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

    TracingCucumberWireProtocolServer::TracingCucumberWireProtocolServer(infra::BoundedString& receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port, CucumberScenarioRequestHandler& scenarioRequestHandler, services::Tracer& tracer)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , tracer(tracer)
        , receiveBuffer(receiveBuffer)
        , scenarioRequestHandler(scenarioRequestHandler)
        , connectionCreator([this](infra::Optional<TracingCucumberWireProtocolConnectionObserver>& value, services::IPAddress address)
              {
                  this->tracer.Trace() << "CucumberWireProtocolServer connection accepted from: " << address;
                  this->receiveBuffer.clear();
                  value.Emplace(this->receiveBuffer, this->scenarioRequestHandler, this->tracer);
              })
    {}
}
