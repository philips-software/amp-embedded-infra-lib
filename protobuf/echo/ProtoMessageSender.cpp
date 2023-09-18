#include "protobuf/echo/ProtoMessageSender.hpp"
#include "protobuf/echo/BufferingStreamWriter.hpp"

namespace services
{
    ProtoMessageSenderBase::ProtoMessageSenderBase(infra::BoundedVector<std::pair<uint32_t, infra::Function<bool(const infra::DataOutputStream& stream, uint32_t& index, bool& retry), 3 * sizeof(uint8_t*)>>>& stack)
        : stack(stack)
    {}

    void ProtoMessageSenderBase::Fill(infra::DataOutputStream output)
    {
        BufferingStreamWriter writer{ buffer, output.Writer() };

        while (!stack.empty())
        {
            //infra::LimitedStreamReaderWithRewinding limitedReader(reader, stack.back().first);
            infra::DataOutputStream::WithErrorPolicy stream{ writer };

            //auto available = limitedReader.Available();

            auto& current = stack.back();

            bool retry = false;
            auto result = current.second(stream, current.first, retry);
            if (result)
                stack.pop_back();
            else if (!retry)
                break;

            if (output.Available() == 0)
                break;

            //if (&current != &stack.front())
            //{
            //    current.first -= available - limitedReader.Available();

            //    if (current.first == 0)
            //        stack.pop_back();
            //}
        }
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoBool, infra::ProtoFormatter& formatter, bool value, uint32_t fieldNumber, bool& retry)
    {
        services::SerializeField(ProtoBool(), formatter, value, fieldNumber);
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoUInt32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber, bool& retry)
    {
        services::SerializeField(ProtoUInt32(), formatter, value, fieldNumber);
        return true;
    }
}
