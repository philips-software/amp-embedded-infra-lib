#ifndef SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP
#define SERVICES_CUCUMBER_WIRE_PROTOCOL_SERVER_HPP

#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/BoundedString.hpp"
#include "services/network/Connection.hpp"
#include "services/network/SingleConnectionListener.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
#include "services/cucumber/CucumberStepStorage.hpp"
#include "services/cucumber/CucumberStep.hpp"
#include "services\cucumber\CucumberWireProtocolParser.hpp"

namespace services
{
    class CucumberWireProtocolConnectionObserver
        : public services::ConnectionObserver
    {
    public:
        CucumberWireProtocolConnectionObserver(const infra::ByteRange receiveBuffer, StepStorage& stepStorage);

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;        

    private:
        CucumberWireProtocolParser cucumberParser;
        infra::ByteRange receiveBuffer;
        infra::BoundedVector<uint8_t> receiveBufferVector;
        infra::ConstByteRange dataBuffer;
        
    };

    class CucumberWireProtocolServer
        : public services::SingleConnectionListener
    {
    public:
        template<size_t BufferSize>
        using WithBuffer = infra::WithStorage<CucumberWireProtocolServer, std::array<uint8_t, BufferSize>>;

        CucumberWireProtocolServer(const infra::ByteRange receiveBuffer, services::ConnectionFactory& connectionFactory, uint16_t port, StepStorage& stepStorage);

    private:
        StepStorage& stepStorage;
        const infra::ByteRange receiveBuffer;
        infra::Creator<services::ConnectionObserver, CucumberWireProtocolConnectionObserver, void(services::IPAddress address)> connectionCreator;
    };
}

#endif
