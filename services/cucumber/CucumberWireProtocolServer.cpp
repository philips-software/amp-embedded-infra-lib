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

        buffer.clear();
        writer = nullptr;
    }

    void CucumberWireProtocolConnectionObserver::DataReceived()
    {
        infra::SharedPtr<infra::StreamReader> reader = Subject().ReceiveStream();
        infra::TextInputStream::WithErrorPolicy stream(*reader);

        if (auto available = stream.Available(); available > buffer.max_size() - buffer.size())
        {
            while (!stream.Empty())
                stream.ContiguousRange();
            // Report error buffer overflow.
        }
        else if (available > 0)
        {
            buffer.resize(buffer.size() + available);
            auto justReceived = buffer.substr(buffer.size() - available);
            stream >> justReceived;

            if (parser.Valid(buffer))
                parser.ParseRequest(buffer);
            controller.HandleRequest(parser);
        }

        Subject().AckReceived();
    }

    CucumberWireProtocolServer::CucumberWireProtocolServer(infra::BoundedString& receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port, CucumberScenarioRequestHandler& scenarioRequestHandler)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , receiveBuffer(receiveBuffer)
        , scenarioRequestHandler(scenarioRequestHandler)
        , connectionCreator([this](infra::Optional<CucumberWireProtocolConnectionObserver>& value, services::IPAddress address) {
            value.Emplace(this->receiveBuffer, this->scenarioRequestHandler);
        })
    {}
}
