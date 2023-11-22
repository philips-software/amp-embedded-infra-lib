#include "infra/syntax/JsonFormatter.hpp"
#include "infra/syntax/EscapeCharacterHelper.hpp"
#include "infra/util/BoundedVector.hpp"

namespace infra
{
    namespace
    {
        const char* charactersToEscape = "<>&'\"";

        std::size_t EscapedCharacterSize(char c)
        {
            std::array<char, 7> shouldBeEscaped = { '"', '\\', '\b', '\f', '\n', '\r', '\t' };
            if (std::any_of(shouldBeEscaped.begin(), shouldBeEscaped.end(), [c](char escape)
                    {
                        return c == escape;
                    }))
                return 2;
            else
                return 6;
        }

        void ReplaceEscapeCharacter(infra::TextOutputStream& stream, char c)
        {
            switch (c)
            {
                case '"':
                    stream << "\\\"";
                    break;
                case '\\':
                    stream << "\\\\";
                    break;
                case '\b':
                    stream << "\\b";
                    break;
                case '\f':
                    stream << "\\f";
                    break;
                case '\n':
                    stream << "\\n";
                    break;
                case '\r':
                    stream << "\\r";
                    break;
                case '\t':
                    stream << "\\t";
                    break;
                default:
                    stream << "\\u" << infra::hex << infra::Width(4, '0') << static_cast<uint8_t>(c);
                    break;
            }
        }

        std::tuple<std::size_t, infra::BoundedConstString> NonEscapedSubString(infra::BoundedConstString string, std::size_t start)
        {
            return infra::NonEscapedSubString(string, start, charactersToEscape);
        }

        void InsertEscapedContent(infra::TextOutputStream& stream, infra::BoundedConstString content)
        {
            infra::InsertEscapedContent(stream, content, charactersToEscape, ReplaceEscapeCharacter);
        }

        void NestedInsert(infra::JsonObjectFormatter& formatter, infra::BoundedConstString key, infra::BoundedConstString path, const infra::JsonValue& valueToMerge)
        {
            infra::JsonObjectFormatter subObjectFormatter{ formatter.SubObject(key) };
            infra::BoundedConstString nextKey = path.substr(0, path.find("/"));
            if (nextKey.size() == path.size())
                subObjectFormatter.Add(infra::JsonString(nextKey), valueToMerge);
            else
                NestedInsert(subObjectFormatter, nextKey, path.substr(nextKey.size() + 1), valueToMerge);
        }

        constexpr std::size_t milliValueWidth = 3;
        constexpr std::size_t nanoValueWidth = 9;
    }

    std::size_t JsonEscapedStringSize(infra::BoundedConstString string)
    {
        std::size_t start = 0;
        std::size_t size = 0;

        while (start != string.size())
        {
            auto [escape, nonEscapedSubString] = NonEscapedSubString(string, start);

            start = escape;
            if (!nonEscapedSubString.empty())
                size += nonEscapedSubString.size();
            if (start != string.size())
            {
                size += EscapedCharacterSize(string[start]);
                ++start;
            }
        }

        return size;
    }

    infra::BoundedConstString JsonSubStringOfMaxEscapedSize(infra::BoundedConstString string, std::size_t maxEscapedSize)
    {
        std::size_t start = 0;
        std::size_t size = 0;

        while (start != string.size() && size < maxEscapedSize)
        {
            infra::BoundedConstString nonEscapedSubString = std::get<1>(NonEscapedSubString(string, start));

            if (!nonEscapedSubString.empty())
            {
                auto increment = std::min(nonEscapedSubString.size(), maxEscapedSize - size);
                size += increment;
                start += increment;
            }
            if (start != string.size())
            {
                auto escapedSize = EscapedCharacterSize(string[start]);
                if (escapedSize > maxEscapedSize - size)
                    break;

                size += escapedSize;
                ++start;
            }
        }

        return string.substr(0, start);
    }

    void Merge(infra::JsonObjectFormatter& formatter, infra::JsonObject& object, infra::BoundedConstString path, const infra::JsonValue& valueToMerge)
    {
        infra::BoundedConstString token;
        infra::BoundedConstString pathRemaining;

        if (!path.empty())
        {
            token = path.substr(0, path.find("/"));
            if (path.size() != token.size())
                pathRemaining = path.substr(token.size() + 1);
        }

        for (auto kv : object)
        {
            if (pathRemaining.empty() && kv.key == token)
            {
                token.clear();
                formatter.Add(kv.key, valueToMerge);
            }
            else if (!pathRemaining.empty() && kv.key == token)
            {
                token.clear();
                infra::JsonObjectFormatter subObjectFormatter{ formatter.SubObject(kv.key) };
                if (kv.value.Is<infra::JsonObject>())
                {
                    infra::JsonObject valueJsonObj = kv.value.Get<infra::JsonObject>();
                    infra::Merge(subObjectFormatter, valueJsonObj, pathRemaining, valueToMerge);
                }
                else
                {
                    auto valueJsonObj = infra::JsonObject("{}");
                    infra::Merge(subObjectFormatter, valueJsonObj, pathRemaining, valueToMerge);
                }
            }
            else
                formatter.Add(kv.key, kv.value);
        }

        if (!token.empty() && !object.HasKey(token))
        {
            if (pathRemaining.empty())
                formatter.Add(infra::JsonString(token), valueToMerge);
            else
                NestedInsert(formatter, token, pathRemaining, valueToMerge);
        }
    }

    JsonObjectFormatter::JsonObjectFormatter(infra::TextOutputStream& stream)
        : stream(&stream)
    {
        *this->stream << "{ ";
    }

    JsonObjectFormatter::JsonObjectFormatter(JsonObjectFormatter&& other) noexcept
        : stream(other.stream)
    {
        other.stream = nullptr;
    }

    JsonObjectFormatter& JsonObjectFormatter::operator=(JsonObjectFormatter&& other) noexcept
    {
        stream = other.stream;
        other.stream = nullptr;

        return *this;
    }

    JsonObjectFormatter::~JsonObjectFormatter()
    {
        if (stream != nullptr)
            *stream << " }";
    }

    void JsonObjectFormatter::Add(const char* tagName, bool tag)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":)" << (tag ? "true" : "false");
    }

    void JsonObjectFormatter::Add(JsonString tagName, bool tag)
    {
        InsertSeparation();
        *stream << '"' << tagName.Raw() << R"(":)" << (tag ? "true" : "false");
    }

    void JsonObjectFormatter::Add(const char* tagName, int8_t tag)
    {
        Add(tagName, static_cast<int64_t>(tag));
    }

    void JsonObjectFormatter::Add(JsonString tagName, int8_t tag)
    {
        Add(tagName, static_cast<int64_t>(tag));
    }

    void JsonObjectFormatter::Add(const char* tagName, uint8_t tag)
    {
        Add(tagName, static_cast<uint64_t>(tag));
    }

    void JsonObjectFormatter::Add(JsonString tagName, uint8_t tag)
    {
        Add(tagName, static_cast<uint64_t>(tag));
    }

    void JsonObjectFormatter::Add(const char* tagName, int16_t tag)
    {
        Add(tagName, static_cast<int64_t>(tag));
    }

    void JsonObjectFormatter::Add(JsonString tagName, int16_t tag)
    {
        Add(tagName, static_cast<int64_t>(tag));
    }

    void JsonObjectFormatter::Add(const char* tagName, uint16_t tag)
    {
        Add(tagName, static_cast<uint64_t>(tag));
    }

    void JsonObjectFormatter::Add(JsonString tagName, uint16_t tag)
    {
        Add(tagName, static_cast<uint64_t>(tag));
    }

    void JsonObjectFormatter::Add(const char* tagName, int32_t tag)
    {
        Add(tagName, static_cast<int64_t>(tag));
    }

    void JsonObjectFormatter::Add(JsonString tagName, int32_t tag)
    {
        Add(tagName, static_cast<int64_t>(tag));
    }

    void JsonObjectFormatter::Add(const char* tagName, uint32_t tag)
    {
        Add(tagName, static_cast<uint64_t>(tag));
    }

    void JsonObjectFormatter::Add(JsonString tagName, uint32_t tag)
    {
        Add(tagName, static_cast<uint64_t>(tag));
    }

    void JsonObjectFormatter::Add(const char* tagName, int64_t tag)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":)" << tag;
    }

    void JsonObjectFormatter::Add(JsonString tagName, int64_t tag)
    {
        InsertSeparation();
        *stream << '"' << tagName.Raw() << R"(":)" << tag;
    }

    void JsonObjectFormatter::Add(const char* tagName, uint64_t tag)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":)" << tag;
    }

    void JsonObjectFormatter::Add(JsonString tagName, uint64_t tag)
    {
        InsertSeparation();
        *stream << '"' << tagName.Raw() << R"(":)" << tag;
    }

    void JsonObjectFormatter::Add(const char* tagName, JsonBiggerInt tag)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":)";
        if (tag.Negative())
            *stream << "-";
        *stream << tag.Value();
    }

    void JsonObjectFormatter::Add(JsonString tagName, JsonBiggerInt tag)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":)";
        if (tag.Negative())
            *stream << "-";
        *stream << tag.Value();
    }

    void JsonObjectFormatter::Add(const char* tagName, const char* tag)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":")";
        InsertEscapedContent(*stream, tag);
        *stream << '"';
    }

    void JsonObjectFormatter::Add(const char* tagName, infra::BoundedConstString tag)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":")";
        InsertEscapedContent(*stream, tag);
        *stream << '"';
    }

    void JsonObjectFormatter::Add(infra::BoundedConstString tagName, infra::BoundedConstString tag)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":")";
        InsertEscapedContent(*stream, tag);
        *stream << '"';
    }

    void JsonObjectFormatter::Add(JsonString tagName, JsonString tag)
    {
        InsertSeparation();
        *stream << '"' << tagName.Raw() << R"(":")" << tag.Raw() << '"';
    }

    void JsonObjectFormatter::Add(JsonString tagName, const JsonObject& tag)
    {
        InsertSeparation();
        *stream << '"' << tagName.Raw() << R"(":)" << tag.ObjectString();
    }

    void JsonObjectFormatter::Add(JsonString tagName, const JsonArray& tag)
    {
        InsertSeparation();
        *stream << '"' << tagName.Raw() << R"(":)" << tag.ObjectString();
    }

    void JsonObjectFormatter::Add(JsonString tagName, const JsonFloat& tag)
    {
        if (tag.NanoFractionalValue() % 1000000 == 0)
            AddFractionalFloat(tagName.Raw(), tag.IntValue(), tag.NanoFractionalValue() / 1000000, tag.Negative(), milliValueWidth);
        else
            AddFractionalFloat(tagName.Raw(), tag.IntValue(), tag.NanoFractionalValue(), tag.Negative(), nanoValueWidth);        
    }

    void JsonObjectFormatter::Add(const infra::JsonKeyValue& keyValue)
    {
        if (keyValue.value.Is<bool>())
            Add(keyValue.key, keyValue.value.Get<bool>());
        else if (keyValue.value.Is<int32_t>())
            Add(keyValue.key, keyValue.value.Get<int32_t>());
        else if (keyValue.value.Is<JsonString>())
            Add(keyValue.key, keyValue.value.Get<JsonString>());
        else if (keyValue.value.Is<JsonObject>())
            Add(keyValue.key, keyValue.value.Get<JsonObject>());
        else if (keyValue.value.Is<JsonArray>())
            Add(keyValue.key, keyValue.value.Get<JsonArray>());
        else
            std::abort();
    }

    void JsonObjectFormatter::Add(JsonString key, const JsonValue& value)
    {
        if (value.Is<bool>())
            Add(key, value.Get<bool>());
        else if (value.Is<int32_t>())
            Add(key, value.Get<int32_t>());
        else if (value.Is<JsonFloat>())
            Add(key, value.Get<JsonFloat>());
        else if (value.Is<JsonString>())
            Add(key, value.Get<JsonString>());
        else if (value.Is<JsonObject>())
            Add(key, value.Get<JsonObject>());
        else if (value.Is<JsonArray>())
            Add(key, value.Get<JsonArray>());
        else
            std::abort();
    }

    void JsonObjectFormatter::AddMilliFloat(const char* tagName, uint32_t intValue, uint32_t milliFractionalValue, bool negative)
    {
        AddFractionalFloat(tagName, intValue, milliFractionalValue, negative, milliValueWidth);
    }

    void JsonObjectFormatter::AddMilliFloat(infra::JsonString tagName, uint32_t intValue, uint32_t milliFractionalValue, bool negative)
    {
        AddFractionalFloat(tagName.Raw(), intValue, milliFractionalValue, negative, milliValueWidth);
    }

    void JsonObjectFormatter::AddSubObject(const char* tagName, infra::BoundedConstString json)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":)" << json;
    }

    JsonObjectFormatter JsonObjectFormatter::SubObject(const char* tagName)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":)";

        return JsonObjectFormatter(*stream);
    }

    JsonObjectFormatter JsonObjectFormatter::SubObject(infra::JsonString tagName)
    {
        InsertSeparation();
        *stream << '"' << tagName.Raw() << R"(":)";

        return JsonObjectFormatter(*stream);
    }

    JsonArrayFormatter JsonObjectFormatter::SubArray(infra::BoundedConstString tagName)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":)";

        return JsonArrayFormatter(*stream);
    }

    JsonStringStream JsonObjectFormatter::AddString(const char* tagName)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":")";

        return JsonStringStream(*stream);
    }

    infra::TextOutputStream JsonObjectFormatter::AddObject(const char* tagName)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":)";

        return *stream;
    }

    bool JsonObjectFormatter::Failed() const
    {
        return stream->Failed();
    }

    void JsonObjectFormatter::AddFractionalFloat(infra::BoundedConstString tagName, uint32_t intValue, uint32_t fractionalValue, bool negative, std::size_t fractionalWidth)
    {
        InsertSeparation();
        *stream << '"' << tagName << R"(":)";

        if (negative)
            *stream << '-';
    
        *stream << intValue << '.' << infra::Width(fractionalWidth, '0') << fractionalValue;
    }

    void JsonObjectFormatter::InsertSeparation()
    {
        if (!empty)
            *stream << ", ";

        empty = false;
    }

    JsonArrayFormatter::JsonArrayFormatter(infra::TextOutputStream& stream)
        : stream(&stream)
    {
        *this->stream << "[ ";
    }

    JsonArrayFormatter::JsonArrayFormatter(JsonArrayFormatter&& other) noexcept
        : stream(other.stream)
    {
        other.stream = nullptr;
    }

    JsonArrayFormatter& JsonArrayFormatter::operator=(JsonArrayFormatter&& other) noexcept
    {
        stream = other.stream;
        other.stream = nullptr;

        return *this;
    }

    JsonArrayFormatter::~JsonArrayFormatter()
    {
        if (stream != nullptr)
            *stream << " ]";
    }

    void JsonArrayFormatter::Add(bool tag)
    {
        InsertSeparation();
        *stream << (tag ? "true" : "false");
    }

    void JsonArrayFormatter::Add(int32_t tag)
    {
        InsertSeparation();
        *stream << tag;
    }

    void JsonArrayFormatter::Add(uint32_t tag)
    {
        InsertSeparation();
        *stream << tag;
    }

    void JsonArrayFormatter::Add(int64_t tag)
    {
        InsertSeparation();
        *stream << tag;
    }

    void JsonArrayFormatter::Add(JsonBiggerInt tag)
    {
        InsertSeparation();
        if (tag.Negative())
            *stream << "-";
        *stream << tag.Value();
    }

    void JsonArrayFormatter::Add(const char* tag)
    {
        InsertSeparation();
        *stream << '"';
        InsertEscapedContent(*stream, tag);
        *stream << '"';
    }

    void JsonArrayFormatter::Add(infra::BoundedConstString tag)
    {
        InsertSeparation();
        *stream << '"';
        InsertEscapedContent(*stream, tag);
        *stream << '"';
    }

    JsonObjectFormatter JsonArrayFormatter::SubObject()
    {
        InsertSeparation();

        return JsonObjectFormatter(*stream);
    }

    JsonArrayFormatter JsonArrayFormatter::SubArray()
    {
        InsertSeparation();

        return JsonArrayFormatter(*stream);
    }

    bool JsonArrayFormatter::Failed() const
    {
        return stream->Failed();
    }

    void JsonArrayFormatter::InsertSeparation()
    {
        if (!empty)
            *stream << ", ";

        empty = false;
    }

    JsonStringStream::JsonStringStream(infra::TextOutputStream& stream)
        : TextOutputStream(stream)
    {}

    JsonStringStream::JsonStringStream(JsonStringStream&& other) noexcept
        : TextOutputStream(std::move(other))
    {
        other.owned = false;
    }

    JsonStringStream& JsonStringStream::operator=(JsonStringStream&& other) noexcept
    {
        owned = other.owned;
        other.owned = false;
        return *this;
    }

    JsonStringStream::~JsonStringStream()
    {
        if (owned)
            *this << '"';
    }
}
