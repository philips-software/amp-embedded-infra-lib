#ifndef PROTOBUF_PROTO_HPP
#define PROTOBUF_PROTO_HPP

#include "infra/syntax/ProtoFormatter.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include <cstdint>

namespace services
{
    struct ProtoBool
    {};

    struct ProtoUInt32
    {};

    struct ProtoInt32
    {};

    struct ProtoUInt64
    {};

    struct ProtoInt64
    {};

    struct ProtoFixed32
    {};

    struct ProtoFixed64
    {};

    struct ProtoSFixed32
    {};

    struct ProtoSFixed64
    {};

    struct ProtoUnboundedString
    {};

    struct ProtoUnboundedBytes
    {};

    template<class T>
    struct ProtoMessage
    {};

    template<class T>
    struct ProtoEnum
    {};

    struct ProtoBytesBase
    {};

    template<std::size_t Max>
    struct ProtoBytes
        : ProtoBytesBase
    {};

    struct ProtoStringBase
    {};

    template<std::size_t Max>
    struct ProtoString
        : ProtoStringBase
    {};

    template<class T>
    struct ProtoRepeatedBase
    {};

    template<std::size_t Max, class T>
    struct ProtoRepeated
        : ProtoRepeatedBase<T>
    {};

    template<class T>
    struct ProtoUnboundedRepeated
    {};

    void SerializeField(ProtoBool, infra::ProtoFormatter& formatter, bool value, uint32_t fieldNumber);
    void SerializeField(ProtoUInt32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber);
    void SerializeField(ProtoInt32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber);
    void SerializeField(ProtoUInt64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber);
    void SerializeField(ProtoInt64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber);
    void SerializeField(ProtoFixed32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber);
    void SerializeField(ProtoFixed64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber);
    void SerializeField(ProtoSFixed32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber);
    void SerializeField(ProtoSFixed64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber);
    void SerializeField(ProtoUnboundedString, infra::ProtoFormatter& formatter, const std::string& value, uint32_t fieldNumber);
    void SerializeField(ProtoUnboundedBytes, infra::ProtoFormatter& formatter, const std::vector<uint8_t>& value, uint32_t fieldNumber);

    template<std::size_t Max, class T, class U>
    void SerializeField(ProtoRepeated<Max, T>, infra::ProtoFormatter& formatter, const infra::BoundedVector<U>& value, uint32_t fieldNumber);
    template<class T, class U>
    void SerializeField(ProtoUnboundedRepeated<T>, infra::ProtoFormatter& formatter, const std::vector<U>& value, uint32_t fieldNumber);
    template<class T>
    void SerializeField(ProtoUnboundedRepeated<T>, infra::ProtoFormatter& formatter, const std::vector<bool>& value, uint32_t fieldNumber);
    template<class T, class U>
    void SerializeField(ProtoMessage<T>, infra::ProtoFormatter& formatter, const U& value, uint32_t fieldNumber);
    template<class T>
    void SerializeField(ProtoEnum<T>, infra::ProtoFormatter& formatter, T value, uint32_t fieldNumber);
    template<std::size_t Max>
    void SerializeField(ProtoBytes<Max>, infra::ProtoFormatter& formatter, const infra::BoundedVector<uint8_t>& value, uint32_t fieldNumber);
    template<std::size_t Max>
    void SerializeField(ProtoString<Max>, infra::ProtoFormatter& formatter, infra::BoundedConstString value, uint32_t fieldNumber);

    void DeserializeField(ProtoBool, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, bool& value);
    void DeserializeField(ProtoUInt32, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, uint32_t& value);
    void DeserializeField(ProtoInt32, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, int32_t& value);
    void DeserializeField(ProtoUInt64, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, uint64_t& value);
    void DeserializeField(ProtoInt64, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, int64_t& value);
    void DeserializeField(ProtoFixed32, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, uint32_t& value);
    void DeserializeField(ProtoFixed64, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, uint64_t& value);
    void DeserializeField(ProtoSFixed32, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, int32_t& value);
    void DeserializeField(ProtoSFixed64, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, int64_t& value);
    void DeserializeField(ProtoUnboundedString, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, std::string& value);
    void DeserializeField(ProtoUnboundedBytes, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, std::vector<uint8_t>& value);

    template<std::size_t Max, class T, class U>
    void DeserializeField(ProtoRepeated<Max, T>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, infra::BoundedVector<U>& value);
    template<class T, class U>
    void DeserializeField(ProtoUnboundedRepeated<T>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, std::vector<U>& value);
    template<class T>
    void DeserializeField(ProtoUnboundedRepeated<T>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, std::vector<bool>& value);
    template<class T, class U>
    void DeserializeField(ProtoMessage<T>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, U& value);
    template<class T>
    void DeserializeField(ProtoEnum<T>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, T& value);
    template<std::size_t Max>
    void DeserializeField(ProtoBytes<Max>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, infra::BoundedVector<uint8_t>& value);
    template<std::size_t Max>
    void DeserializeField(ProtoBytes<Max>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, infra::ConstByteRange& value);
    template<std::size_t Max>
    void DeserializeField(ProtoString<Max>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, infra::BoundedString& value);
    template<std::size_t Max>
    void DeserializeField(ProtoString<Max>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, infra::BoundedConstString& value);

    void DeserializeField(ProtoBool, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, bool& value);
    void DeserializeField(ProtoUInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value);
    void DeserializeField(ProtoInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value);
    void DeserializeField(ProtoUInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value);
    void DeserializeField(ProtoInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value);
    void DeserializeField(ProtoFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value);
    void DeserializeField(ProtoFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value);
    void DeserializeField(ProtoSFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value);
    void DeserializeField(ProtoSFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value);

    template<class Proto>
    struct MessageDepth
    {
        static constexpr uint32_t value = 0;
    };

    template<>
    struct MessageDepth<ProtoUnboundedString>
    {
        static constexpr uint32_t value = 1;
    };

    template<>
    struct MessageDepth<ProtoUnboundedBytes>
    {
        static constexpr uint32_t value = 1;
    };

    template<class T>
    struct MessageDepth<ProtoUnboundedRepeated<T>>
    {
        static constexpr uint32_t value = 1;
    };

    template<std::size_t Max>
    struct MessageDepth<ProtoString<Max>>
    {
        static constexpr uint32_t value = 1;
    };

    template<std::size_t Max>
    struct MessageDepth<ProtoBytes<Max>>
    {
        static constexpr uint32_t value = 1;
    };

    template<std::size_t Max, class T>
    struct MessageDepth<ProtoRepeated<Max, T>>
    {
        static constexpr uint32_t value = 1;
    };

    template<uint32_t... V>
    struct Max;

    template<uint32_t V>
    struct Max<V>
    {
        static constexpr uint32_t value = V;
    };

    template<uint32_t V, uint32_t... Tail>
    struct Max<V, Tail...>
    {
        static constexpr uint32_t value = std::max(V, Max<Tail...>::value);
    };

    template<class T, class I>
    struct MaxFieldsDepth;

    template<class T, std::size_t... I>
    struct MaxFieldsDepth<T, std::integer_sequence<std::size_t, I...>>
    {
        static constexpr uint32_t value = Max<MessageDepth<typename T::template ProtoType<I>>::value...>::value;
    };

    template<class T>
    struct MessageDepth<ProtoMessage<T>>
    {
        static constexpr uint32_t value = MaxFieldsDepth<T, std::make_index_sequence<T::numberOfFields>>::value + 1;
    };

    ////    Implementation    ////

    inline void SerializeField(ProtoBool, infra::ProtoFormatter& formatter, bool value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(value, fieldNumber);
    }

    inline void SerializeField(ProtoUInt32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(value, fieldNumber);
    }

    inline void SerializeField(ProtoInt32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(value, fieldNumber);
    }

    inline void SerializeField(ProtoUInt64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(value, fieldNumber);
    }

    inline void SerializeField(ProtoInt64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(value, fieldNumber);
    }

    inline void SerializeField(ProtoFixed32, infra::ProtoFormatter& formatter, uint32_t value, uint32_t fieldNumber)
    {
        formatter.PutFixed32Field(value, fieldNumber);
    }

    inline void SerializeField(ProtoFixed64, infra::ProtoFormatter& formatter, uint64_t value, uint32_t fieldNumber)
    {
        formatter.PutFixed64Field(value, fieldNumber);
    }

    inline void SerializeField(ProtoSFixed32, infra::ProtoFormatter& formatter, int32_t value, uint32_t fieldNumber)
    {
        formatter.PutFixed32Field(static_cast<uint32_t>(value), fieldNumber);
    }

    inline void SerializeField(ProtoSFixed64, infra::ProtoFormatter& formatter, int64_t value, uint32_t fieldNumber)
    {
        formatter.PutFixed64Field(static_cast<uint64_t>(value), fieldNumber);
    }

    inline void SerializeField(ProtoUnboundedString, infra::ProtoFormatter& formatter, const std::string& value, uint32_t fieldNumber)
    {
        formatter.PutStringField(value, fieldNumber);
    }

    inline void SerializeField(ProtoUnboundedBytes, infra::ProtoFormatter& formatter, const std::vector<uint8_t>& value, uint32_t fieldNumber)
    {
        formatter.PutBytesField(value, fieldNumber);
    }

    template<std::size_t Max, class T, class U>
    void SerializeField(ProtoRepeated<Max, T>, infra::ProtoFormatter& formatter, const infra::BoundedVector<U>& value, uint32_t fieldNumber)
    {
        for (auto& v : value)
            SerializeField(T(), formatter, v, fieldNumber);
    }

    template<class T, class U>
    void SerializeField(ProtoUnboundedRepeated<T>, infra::ProtoFormatter& formatter, const std::vector<U>& value, uint32_t fieldNumber)
    {
        for (auto& v : value)
            SerializeField(T(), formatter, v, fieldNumber);
    }

    template<class T>
    void SerializeField(ProtoUnboundedRepeated<T>, infra::ProtoFormatter& formatter, const std::vector<bool>& value, uint32_t fieldNumber)
    {
        for (auto v : value)
            SerializeField(T(), formatter, v, fieldNumber);
    }

    template<class T, class U>
    void SerializeField(ProtoMessage<T>, infra::ProtoFormatter& formatter, const U& value, uint32_t fieldNumber)
    {
        infra::ProtoLengthDelimitedFormatter nestedMessage(formatter, fieldNumber);
        value.Serialize(formatter);
    }

    template<class T>
    void SerializeField(ProtoEnum<T>, infra::ProtoFormatter& formatter, T value, uint32_t fieldNumber)
    {
        formatter.PutVarIntField(static_cast<uint64_t>(value), fieldNumber);
    }

    template<std::size_t Max>
    void SerializeField(ProtoBytes<Max>, infra::ProtoFormatter& formatter, const infra::BoundedVector<uint8_t>& value, uint32_t fieldNumber)
    {
        formatter.PutBytesField(infra::MakeRange(value), fieldNumber);
    }

    template<std::size_t Max>
    void SerializeField(ProtoString<Max>, infra::ProtoFormatter& formatter, infra::BoundedConstString value, uint32_t fieldNumber)
    {
        formatter.PutStringField(value, fieldNumber);
    }

    inline void DeserializeField(ProtoBool, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, bool& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = field.Get<uint64_t>() != 0;
    }

    inline void DeserializeField(ProtoUInt32, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, uint32_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = static_cast<uint32_t>(field.Get<uint64_t>());
    }

    inline void DeserializeField(ProtoInt32, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, int32_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = static_cast<int32_t>(field.Get<uint64_t>());
    }

    inline void DeserializeField(ProtoUInt64, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, uint64_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = field.Get<uint64_t>();
    }

    inline void DeserializeField(ProtoInt64, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, int64_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = static_cast<int64_t>(field.Get<uint64_t>());
    }

    inline void DeserializeField(ProtoFixed32, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, uint32_t& value)
    {
        parser.ReportFormatResult(field.Is<uint32_t>());
        if (field.Is<uint32_t>())
            value = field.Get<uint32_t>();
    }

    inline void DeserializeField(ProtoFixed64, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, uint64_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = field.Get<uint64_t>();
    }

    inline void DeserializeField(ProtoSFixed32, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, int32_t& value)
    {
        parser.ReportFormatResult(field.Is<uint32_t>());
        if (field.Is<uint32_t>())
            value = static_cast<int32_t>(field.Get<uint32_t>());
    }

    inline void DeserializeField(ProtoSFixed64, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, int64_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = static_cast<int64_t>(field.Get<uint64_t>());
    }

    inline void DeserializeField(ProtoUnboundedString, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, std::string& value)
    {
        parser.ReportFormatResult(field.Is<infra::ProtoLengthDelimited>());
        if (field.Is<infra::ProtoLengthDelimited>())
            value = field.Get<infra::ProtoLengthDelimited>().GetStdString();
    }

    inline void DeserializeField(ProtoUnboundedBytes, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, std::vector<uint8_t>& value)
    {
        parser.ReportFormatResult(field.Is<infra::ProtoLengthDelimited>());
        if (field.Is<infra::ProtoLengthDelimited>())
            value = field.Get<infra::ProtoLengthDelimited>().GetUnboundedBytes();
    }

    template<std::size_t Max, class T, class U>
    void DeserializeField(ProtoRepeated<Max, T>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, infra::BoundedVector<U>& value)
    {
        parser.ReportFormatResult(!value.full());
        if (!value.full())
        {
            value.emplace_back();
            DeserializeField(T(), parser, field, value.back());
        }
    }

    template<class T, class U>
    void DeserializeField(ProtoUnboundedRepeated<T>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, std::vector<U>& value)
    {
        value.emplace_back();
        DeserializeField(T(), parser, field, value.back());
    }

    template<class T>
    void DeserializeField(ProtoUnboundedRepeated<T>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, std::vector<bool>& value)
    {
        bool result{};
        DeserializeField(T(), parser, field, result);
        value.push_back(result);
    }

    template<class T, class U>
    void DeserializeField(ProtoMessage<T>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, U& value)
    {
        parser.ReportFormatResult(field.Is<infra::ProtoLengthDelimited>());
        if (field.Is<infra::ProtoLengthDelimited>())
        {
            infra::ProtoParser nestedParser = field.Get<infra::ProtoLengthDelimited>().Parser();
            value.Deserialize(nestedParser);
        }
    }

    template<class T>
    void DeserializeField(ProtoEnum<T>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, T& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = static_cast<T>(field.Get<uint64_t>());
    }

    template<std::size_t Max>
    void DeserializeField(ProtoBytes<Max>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, infra::BoundedVector<uint8_t>& value)
    {
        parser.ReportFormatResult(field.Is<infra::ProtoLengthDelimited>());
        if (field.Is<infra::ProtoLengthDelimited>())
            field.Get<infra::ProtoLengthDelimited>().GetBytes(value);
    }

    template<std::size_t Max>
    void DeserializeField(ProtoBytes<Max>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, infra::ConstByteRange& value)
    {
        parser.ReportFormatResult(field.Is<infra::ProtoLengthDelimited>());
        if (field.Is<infra::ProtoLengthDelimited>())
            field.Get<infra::ProtoLengthDelimited>().GetBytesReference(value);
    }

    template<std::size_t Max>
    void DeserializeField(ProtoString<Max>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, infra::BoundedString& value)
    {
        parser.ReportFormatResult(field.Is<infra::ProtoLengthDelimited>());
        if (field.Is<infra::ProtoLengthDelimited>())
            field.Get<infra::ProtoLengthDelimited>().GetString(value);
    }

    template<std::size_t Max>
    void DeserializeField(ProtoString<Max>, infra::ProtoParser& parser, infra::ProtoParser::FieldVariant& field, infra::BoundedConstString& value)
    {
        parser.ReportFormatResult(field.Is<infra::ProtoLengthDelimited>());
        if (field.Is<infra::ProtoLengthDelimited>())
            field.Get<infra::ProtoLengthDelimited>().GetStringReference(value);
    }

    inline void DeserializeField(ProtoBool, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, bool& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = field.Get<uint64_t>() != 0;
    }

    inline void DeserializeField(ProtoUInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = static_cast<uint32_t>(field.Get<uint64_t>());
    }

    inline void DeserializeField(ProtoInt32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = static_cast<int32_t>(field.Get<uint64_t>());
    }

    inline void DeserializeField(ProtoUInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = field.Get<uint64_t>();
    }

    inline void DeserializeField(ProtoInt64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = static_cast<int64_t>(field.Get<uint64_t>());
    }

    inline void DeserializeField(ProtoFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint32_t& value)
    {
        parser.ReportFormatResult(field.Is<uint32_t>());
        if (field.Is<uint32_t>())
            value = field.Get<uint32_t>();
    }

    inline void DeserializeField(ProtoFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, uint64_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = field.Get<uint64_t>();
    }

    inline void DeserializeField(ProtoSFixed32, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int32_t& value)
    {
        parser.ReportFormatResult(field.Is<uint32_t>());
        if (field.Is<uint32_t>())
            value = static_cast<int32_t>(field.Get<uint32_t>());
    }

    inline void DeserializeField(ProtoSFixed64, infra::ProtoParser& parser, infra::ProtoParser::PartialFieldVariant& field, int64_t& value)
    {
        parser.ReportFormatResult(field.Is<uint64_t>());
        if (field.Is<uint64_t>())
            value = static_cast<int64_t>(field.Get<uint64_t>());
    }
}

#endif
