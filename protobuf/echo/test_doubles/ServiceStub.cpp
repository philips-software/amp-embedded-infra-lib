#include "protobuf/echo/test_doubles/ServiceStub.hpp"

namespace services
{
    void ServiceStub::Handle(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, services::EchoErrorPolicy& errorPolicy)
    {
        infra::ProtoParser parser(contents.Parser());

        switch (methodId)
        {
            uint32_t value;

            case idMethod:
            {
                while (!parser.Empty())
                {
                    infra::ProtoParser::Field field = parser.GetField();

                    switch (field.second)
                    {
                        case 1:
                            DeserializeField(services::ProtoUInt32(), parser, field.first, value);
                            break;
                        default:
                            if (field.first.Is<infra::ProtoLengthDelimited>())
                                field.first.Get<infra::ProtoLengthDelimited>().SkipEverything();
                            break;
                    }
                }

                if (!parser.FormatFailed())
                    Method(value);
                break;
            }
            default:
                errorPolicy.MethodNotFound(serviceId, methodId);
                contents.SkipEverything();
        }
    }

    ServiceStubProxy::ServiceStubProxy(services::Echo& echo)
        : services::ServiceProxy(echo, maxMessageSize)
    {}

    void ServiceStubProxy::Method(uint32_t value)
    {
        infra::DataOutputStream::WithErrorPolicy stream(Rpc().SendStreamWriter());
        infra::ProtoFormatter formatter(stream);
        formatter.PutVarInt(serviceId);
        {
            infra::ProtoLengthDelimitedFormatter argumentFormatter = formatter.LengthDelimitedFormatter(idMethod);
            SerializeField(services::ProtoUInt32(), formatter, value, 1);
        }
        Rpc().Send();
    }
}
