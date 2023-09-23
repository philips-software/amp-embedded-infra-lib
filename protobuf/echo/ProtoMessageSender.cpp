#include "protobuf/echo/ProtoMessageSender.hpp"
#include "protobuf/echo/BufferingStreamWriter.hpp"

namespace services
{
    ProtoMessageSenderBase::ProtoMessageSenderBase(infra::BoundedVector<std::pair<uint32_t, infra::Function<bool(infra::DataOutputStream& stream, uint32_t& index, bool& retry, const infra::StreamWriter& finalWriter), 3 * sizeof(uint8_t*)>>>& stack)
        : stack(stack)
    {}

    void ProtoMessageSenderBase::Fill(infra::DataOutputStream output)
    {
        BufferingStreamWriter writer{ buffer, output.Writer() };

        while (!stack.empty())
        {
            infra::DataOutputStream::WithErrorPolicy stream{ writer };

            auto& [index, callback] = stack.back();
            bool retry = false;
            auto result = callback(stream, index, retry, output.Writer());
            if (result)
                stack.pop_back();
            else if (!retry)
                break;
        }
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoBool, infra::ProtoFormatter& formatter, bool value, uint32_t fieldNumber, [[maybe_unused]] const bool& retry) const
    {
        services::SerializeField(ProtoBool(), formatter, value, fieldNumber);
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoUInt32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber, [[maybe_unused]] const bool& retry) const
    {
        services::SerializeField(ProtoUInt32(), formatter, value, fieldNumber);
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoInt32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber, [[maybe_unused]] const bool& retry) const
    {
        services::SerializeField(ProtoInt32(), formatter, value, fieldNumber);
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoUInt64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber, [[maybe_unused]] const bool& retry) const
    {
        services::SerializeField(ProtoUInt64(), formatter, value, fieldNumber);
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoInt64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber, [[maybe_unused]] const bool& retry) const
    {
        services::SerializeField(ProtoInt64(), formatter, value, fieldNumber);
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoFixed32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber, [[maybe_unused]] const bool& retry) const
    {
        services::SerializeField(ProtoFixed32(), formatter, value, fieldNumber);
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoSFixed32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber, [[maybe_unused]] const bool& retry) const
    {
        services::SerializeField(ProtoSFixed32(), formatter, value, fieldNumber);
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoFixed64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber, [[maybe_unused]] const bool& retry) const
    {
        services::SerializeField(ProtoFixed64(), formatter, value, fieldNumber);
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoSFixed64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber, [[maybe_unused]] const bool& retry) const
    {
        services::SerializeField(ProtoSFixed64(), formatter, value, fieldNumber);
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoStringBase, infra::ProtoFormatter& formatter, const infra::BoundedString& value, uint32_t fieldNumber, bool& retry) const
    {
        formatter.PutVarInt((fieldNumber << 3) | 2);
        formatter.PutVarInt(value.size());

        stack.emplace_back(0, [&value](infra::DataOutputStream& stream, uint32_t& index, const bool&, const infra::StreamWriter&)
            {
                auto range = infra::Head(infra::DiscardHead(infra::StringAsByteRange(value), index), stream.Available());
                stream << range;
                index += range.size();
                return index == value.size();
            });

        retry = true;
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoUnboundedString, infra::ProtoFormatter& formatter, const std::string& value, uint32_t fieldNumber, bool& retry) const
    {
        formatter.PutVarInt((fieldNumber << 3) | 2);
        formatter.PutVarInt(value.size());

        stack.emplace_back(0, [&value](infra::DataOutputStream& stream, uint32_t& index, const bool&, const infra::StreamWriter&)
            {
                auto range = infra::Head(infra::DiscardHead(infra::StdStringAsByteRange(value), index), stream.Available());
                stream << range;
                index += range.size();
                return index == value.size();
            });

        retry = true;
        return true;
    }

    bool ProtoMessageSenderBase::SerializeField(ProtoBytesBase, infra::ProtoFormatter& formatter, const infra::BoundedVector<uint8_t>& value, uint32_t fieldNumber, bool& retry) const
    {
        formatter.PutVarInt((fieldNumber << 3) | 2);
        formatter.PutVarInt(value.size());

        stack.emplace_back(0, [&value](infra::DataOutputStream& stream, uint32_t& index, const bool&, const infra::StreamWriter&)
            {
                auto range = infra::Head(infra::DiscardHead(infra::MakeRange(value), index), stream.Available());
                stream << range;
                index += range.size();
                return index == value.size();
            });

        retry = true;
        return true;
    }
}
