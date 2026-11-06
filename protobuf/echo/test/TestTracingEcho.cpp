#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/StdStringOutputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "protobuf/echo/EchoOnStreams.hpp"
#include "protobuf/echo/ServiceForwarder.hpp"
#include "protobuf/echo/TracingEcho.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "services/tracer/Tracer.hpp"
#include "gmock/gmock.h"
#include <array>
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
        using services::EchoOnStreams::SendStreamAvailable;

        void ResetFromTransport()
        {
            Reset();
        }
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
            tracer.Continue() << methodId;
            contents.SkipEverything();
        }
    };

    class FragmentingEcho
        : public services::EchoOnStreams
    {
    public:
        using services::EchoOnStreams::EchoOnStreams;
        using services::EchoOnStreams::SendStreamAvailable;

    protected:
        void RequestSendStream(std::size_t) override
        {
            ++grants;
            SendStreamAvailable(writer.Emplace(infra::MakeRange(buffer)));
        }

    public:
        std::size_t grants = 0;

    private:
        std::array<uint8_t, 16> buffer{};
        infra::SharedOptional<infra::ByteOutputStreamWriter> writer;
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

TEST_F(TracingEchoTest, reset_discards_partially_sent_trace_before_next_message)
{
    services::ServiceStubProxy proxy{ *echo };
    std::array<uint8_t, 4> data{};
    infra::SharedOptional<infra::ByteOutputStreamWriter> writer;

    EXPECT_CALL(*echo, RequestSendStream(testing::_)).Times(2);
    proxy.RequestSend([&proxy]()
        {
            proxy.Method(42);
        });
    echo->SendStreamAvailable(writer.Emplace(infra::MakeRange(data)));

    echo->ResetFromTransport();

    EXPECT_CALL(*echo, RequestSendStream(testing::_));
    proxy.RequestSend([&proxy]()
        {
            proxy.MethodNoParameter();
        });
    echo->SendStreamAvailable(writer.Emplace(infra::MakeRange(data)));

    EXPECT_THAT(stream.Storage(), testing::Eq("\r\n< 3"));
}

TEST(TracingEchoForwardingTest, generic_forwarder_sends_fragmented_message_over_traced_echo)
{
    infra::StdStringOutputStream::WithStorage trace;
    services::TracerToStream tracer{ trace };
    testing::StrictMock<services::EchoErrorPolicyMock> errorPolicy;
    services::MethodSerializerFactory::ForServices<services::ServiceStub>::AndProxies<services::ServiceStubProxy> fromFactory;
    services::MethodSerializerFactory::ForServices<services::ServiceStub>::AndProxies<services::ServiceStubProxy> toFactory;
    application::EchoSingleLoopback from{ fromFactory, errorPolicy };
    services::TracingEchoOnStreamsDescendant<FragmentingEcho> to{ toFactory, errorPolicy, tracer };
    ServiceStubTracer serviceTracer;
    to.AddServiceTracer(serviceTracer);
    services::ServiceForwarder forwarder{ from, services::ServiceStub::serviceId, to };
    services::ServiceStubProxy request{ from };
    std::array<uint8_t, 64> data{};

    request.RequestSend([&]()
        {
            request.MethodBytes(infra::MakeRange(data));
        });

    EXPECT_GT(to.grants, 1U);
    EXPECT_THAT(trace.Storage(), testing::Eq("\r\n< 4"));
}
