#include "services/cucumber/CucumberWireProtocolServer.hpp"

namespace services
{
    CucumberWireProtocolConnectionObserver::CucumberWireProtocolConnectionObserver(infra::BoundedString& buffer, CucumberScenarioRequestHandler& scenarioRequestHandler)
        : buffer(buffer)
        , controller(*this, scenarioRequestHandler)
        , formatter(parser, controller)
    {}

    void CucumberWireProtocolConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::TextOutputStream::WithErrorPolicy stream(*writer);
        formatter.FormatResponse(stream);

        writer = nullptr;
    }

    void CucumberWireProtocolConnectionObserver::DataReceived()
    {
        buffer.clear();

        infra::SharedPtr<infra::StreamReader> reader = Subject().ReceiveStream();
        infra::TextInputStream::WithErrorPolicy stream(*reader);

        auto available = stream.Available();

        really_assert(available <= buffer.max_size());

        if (available > 0)
        {
            buffer.resize(available);
            stream >> buffer;
            Subject().AckReceived();

            if (parser.Valid(buffer))
                parser.ParseRequest(buffer);
            controller.HandleRequest(parser);
        }
    }

    CucumberWireProtocolServer::CucumberWireProtocolServer(infra::BoundedString& receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port, CucumberScenarioRequestHandler& scenarioRequestHandler)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , receiveBuffer(receiveBuffer)
        , scenarioRequestHandler(scenarioRequestHandler)
        , connectionCreator([this](infra::Optional<CucumberWireProtocolConnectionObserver>& value, services::IPAddress address) {
            this->receiveBuffer.clear();
            value.Emplace(this->receiveBuffer, this->scenarioRequestHandler);
        })
    {}
}
