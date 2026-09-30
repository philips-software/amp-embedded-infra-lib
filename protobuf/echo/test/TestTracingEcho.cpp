#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/SharedPtr.hpp"
#include "protobuf/echo/EchoOnStreams.hpp"
#include "protobuf/echo/Serialization.hpp"
#include "protobuf/echo/TracingEcho.hpp"
#include "services/tracer/Tracer.hpp"
#include "gmock/gmock.h"

namespace services
{
    class ServiceWithDeserializer
        : public Service
    {
    public:
        ServiceWithDeserializer(Echo& echo, infra::SharedPtr<MethodDeserializer>&& deserializer)
            : Service(echo)
            , deserializer(std::move(deserializer))
        {}

        bool AcceptsService(uint32_t) const override
        {
            return true;
        }

        infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t, uint32_t, uint32_t, const EchoErrorPolicy&) override
        {
            return std::move(deserializer);
        }

    private:
        infra::SharedPtr<MethodDeserializer> deserializer;
    };

    class EchoOnStreamsStub
        : public EchoOnStreams
    {
    public:
        using EchoOnStreams::DataReceived;
        using EchoOnStreams::EchoOnStreams;
        using EchoOnStreams::Reset;

    protected:
        void RequestSendStream(std::size_t) override
        {}
    };

    using TracingEchoOnStreamsStub = TracingEchoOnStreamsDescendant<EchoOnStreamsStub>;
}

TEST(TracingEchoOnStreamsTest, reset_releases_in_flight_deserializer)
{
    infra::StringOutputStream::WithStorage<32> trace;
    services::TracerToStream tracer(trace);
    services::MethodSerializerFactory::OnHeap serializerFactory;
    services::TracingEchoOnStreamsStub tracingEcho(serializerFactory, services::echoErrorPolicyAbortOnMessageFormatError, tracer);
    services::MethodDeserializerDummy deserializer(tracingEcho);
    infra::AccessedBySharedPtr releaseAssert{ infra::emptyFunction };
    services::ServiceWithDeserializer service(tracingEcho, releaseAssert.MakeShared(deserializer));
    infra::SharedOptional<infra::ByteInputStreamReader> reader;
    std::array<uint8_t, 3> data{ 1, (1 << 3) | 2, 64 };

    tracingEcho.DataReceived(reader.Emplace(infra::MakeRange(data)));
    ASSERT_TRUE(releaseAssert.Referenced());

    tracingEcho.Reset();

    EXPECT_FALSE(releaseAssert.Referenced());
    if (releaseAssert.Referenced())
        tracingEcho.ServiceDone();
}
