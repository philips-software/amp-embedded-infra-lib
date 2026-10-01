#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/StdStringOutputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "protobuf/echo/EchoOnStreams.hpp"
#include "protobuf/echo/TracingEcho.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/tracer/Tracer.hpp"
#include "gmock/gmock.h"
#include <optional>

namespace
{
    class EchoOnStreamsStub
        : public services::EchoOnStreams
    {
    public:
        using services::EchoOnStreams::EchoOnStreams;

        MOCK_METHOD(void, RequestSendStream, (std::size_t size), (override));

        using services::EchoOnStreams::DataReceived;
        using services::EchoOnStreams::Reset;
    };

    class ServiceStubTracer
        : public services::ServiceTracer
    {
    public:
        ServiceStubTracer()
            : services::ServiceTracer(services::ServiceStub::serviceId)
        {}

        void TraceMethod(uint32_t methodId, infra::ProtoLengthDelimited& contents, services::Tracer& tracer) const override
        {
            contents.SkipEverything();
        }
    };
}

class TracingEchoTest
    : public testing::Test
{
public:
    TracingEchoTest()
    {
        echo.emplace(serializerFactory, errorPolicy, tracer);
        echo->AddServiceTracer(serviceTracer);
        service.emplace(*echo);
    }

    ~TracingEchoTest() override
    {
        Destroy();
    }

    void ReceivePartialMethod()
    {
        echo->DataReceived(reader.Emplace(infra::MakeRange(partialMessage)));
    }

    void Destroy()
    {
        service.reset();
        echo.reset();
    }

    infra::StdStringOutputStream::WithStorage stream;
    services::TracerToStream tracer{ stream };
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    services::MethodSerializerFactory::ForServices<services::ServiceStub>::AndProxies<services::ServiceStubProxy> serializerFactory;
    ServiceStubTracer serviceTracer;
    std::optional<testing::StrictMock<services::TracingEchoOnStreamsDescendant<EchoOnStreamsStub>>> echo;
    std::optional<testing::StrictMock<services::ServiceStub>> service;

    infra::SharedOptional<infra::ByteInputStreamReader> reader;
    std::array<uint8_t, 4> partialMessage{ services::ServiceStub::serviceId, (services::ServiceStub::idMethod << 3) | 2, 4, 1 << 3 };
};

TEST_F(TracingEchoTest, destruction_with_partially_received_method_does_not_abort)
{
    ReceivePartialMethod();

    Destroy();
}

TEST_F(TracingEchoTest, destruction_after_reset_with_partially_received_method_does_not_abort)
{
    ReceivePartialMethod();

    echo->Reset();
    Destroy();
}
