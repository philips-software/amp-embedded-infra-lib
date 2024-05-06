#include "infra/syntax/Json.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include <algorithm>
#include <cctype>

namespace
{
    template<class Iterator>
    bool Equal(Iterator leftBegin, Iterator leftEnd, Iterator rightBegin, Iterator rightEnd)
    {
        while (leftBegin != leftEnd && rightBegin != rightEnd)
        {
            if (*leftBegin != *rightBegin)
                return false;

            ++leftBegin;
            ++rightBegin;
        }

        return leftBegin == leftEnd && rightBegin == rightEnd;
    }

    template<class IntType>
    IntType ToInt(infra::BoundedConstString string, bool sign)
    {
        IntType value{};
        for (auto c : string.substr(sign ? 1 : 0))
            value = value * 10 + c - '0';

        return value;
    }

    constexpr std::size_t nanoValueWidth = 9;
}

namespace infra
{
    JsonStringIterator::JsonStringIterator(infra::BoundedConstString::const_iterator position, infra::BoundedConstString::const_iterator end)
        : position(position)
        , end(end)
    {}

    char JsonStringIterator::operator*() const
    {
        if (*position == '\\')
        {
            auto next = std::next(position);
            // A JSON string cannot end in a backslash, since that backslash would have escaped the closing quotes, and without closing quotes
            // no valid JSON string would have been produced
            assert(next != end);

            switch (*next++)
            {
                case '"':
                    return '"';
                case '\\':
                    return '\\';
                case 'b':
                    return '\b';
                case 'f':
                    return '\f';
                case 'n':
                    return '\n';
                case 'r':
                    return '\r';
                case 't':
                    return '\t';
                case 'u':
                {
                    char result = 0;

                    for (int skipCode = 0; skipCode != 4 && next != end; ++skipCode, ++next)
                    {
                        if (*next >= '0' && *next <= '9')
                        {
                            result *= 16;
                            result += *next - '0';
                        }
                        else if (*next >= 'a' && *next <= 'f')
                        {
                            result *= 16;
                            result += *next - 'a' + 10;
                        }
                        else if (*next >= 'A' && *next <= 'F')
                        {
                            result *= 16;
                            result += *next - 'A' + 10;
                        }
                        else
                            break;
                    }

                    return result;
                }
                default:
                    return *std::prev(next);
            }
        }
        else
            return *position;
    }

    JsonStringIterator& JsonStringIterator::operator++()
    {
        if (*position == '\\')
        {
            ++position;

            if (position != end)
            {
                if (*position == 'u')
                {
                    ++position;
                    for (int skipCode = 0; skipCode != 4 && position != end; ++skipCode, ++position)
                        if ((*position < '0' || *position > '9') && (*position < 'a' || *position > 'f') && (*position < 'A' || *position > 'F'))
                            break;
                }
                else
                    ++position;
            }
        }
        else
            ++position;

        return *this;
    }

    JsonStringIterator JsonStringIterator::operator++(int)
    {
        JsonStringIterator result(*this);
        ++*this;
        return result;
    }

    bool JsonStringIterator::operator==(const JsonStringIterator& other) const
    {
        return position == other.position;
    }

    bool JsonStringIterator::operator!=(const JsonStringIterator& other) const
    {
        return !(*this == other);
    }

    JsonString::JsonString(infra::BoundedConstString source)
        : source(source)
    {}

    JsonString::JsonString(const char* source)
        : source(source)
    {}

    bool JsonString::operator==(const JsonString& other) const
    {
        return source == other.source;
    }

    bool JsonString::operator!=(const JsonString& other) const
    {
        return !(*this == other);
    }

    bool JsonString::operator==(infra::BoundedConstString other) const
    {
        auto x = begin();
        auto y = other.begin();

        while (x != end() && y != other.end())
        {
            if (*x != *y)
                return false;

            ++x;
            ++y;
        }

        return x == end() && y == other.end();
    }

    bool JsonString::operator!=(infra::BoundedConstString other) const
    {
        return !(*this == other);
    }

    bool operator==(infra::BoundedConstString x, JsonString y)
    {
        return y == x;
    }

    bool operator!=(infra::BoundedConstString x, JsonString y)
    {
        return y != x;
    }

    bool JsonString::operator==(const char* other) const
    {
        auto x = begin();
        auto y = other;

        while (x != end() && *y != 0)
        {
            if (*x != *y)
                return false;

            ++x;
            ++y;
        }

        return x == end() && *y == 0;
    }

    bool JsonString::operator!=(const char* other) const
    {
        return !(*this == other);
    }

    bool operator==(const char* x, JsonString y)
    {
        return y == x;
    }

    bool operator!=(const char* x, JsonString y)
    {
        return y != x;
    }

    bool JsonString::empty() const
    {
        return size() == 0;
    }

    std::size_t JsonString::size() const
    {
        return std::distance(begin(), end());
    }

    JsonStringIterator JsonString::begin() const
    {
        return JsonStringIterator(source.begin(), source.end());
    }

    JsonStringIterator JsonString::end() const
    {
        return JsonStringIterator(source.end(), source.end());
    }

    infra::BoundedConstString JsonString::Raw() const
    {
        return source;
    }

    void JsonString::ToString(infra::BoundedString& result) const
    {
        result.clear();
        AppendTo(result);
    }

    void JsonString::AppendTo(infra::BoundedString& result) const
    {
        for (auto c : *this)
        {
            result.push_back(c);
            if (result.full())
                return;
        }
    }

#ifdef EMIL_HOST_BUILD
    std::string JsonString::ToStdString() const
    {
        std::string result;

        for (auto c : *this)
            result.push_back(c);

        return result;
    }
#endif

    JsonFloat::JsonFloat(uint64_t intValue, uint32_t nanoFractionalValue, bool negative)
        : intValue(intValue)
        , nanoFractionalValue(nanoFractionalValue)
        , negative(negative)
    {}

    bool JsonFloat::operator==(const JsonFloat& other) const
    {
        return intValue == other.intValue && nanoFractionalValue == other.nanoFractionalValue;
    }

    bool JsonFloat::operator!=(const JsonFloat& other) const
    {
        return !(*this == other);
    }

    uint64_t JsonFloat::IntValue() const
    {
        return intValue;
    }

    uint32_t JsonFloat::NanoFractionalValue() const
    {
        return nanoFractionalValue;
    }

    bool JsonFloat::Negative() const
    {
        return negative;
    }

    JsonBiggerInt::JsonBiggerInt(uint64_t value, bool negative)
        : value(value)
        , negative(negative)
    {}

    bool JsonBiggerInt::operator==(const JsonBiggerInt& other) const
    {
        return value == other.value && negative == other.negative;
    }

    bool JsonBiggerInt::operator!=(const JsonBiggerInt& other) const
    {
        return !(*this == other);
    }

    uint64_t JsonBiggerInt::Value() const
    {
        return value;
    }

    bool JsonBiggerInt::Negative() const
    {
        return negative;
    }

    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, JsonString value)
    {
        for (auto c : value)
            stream << c;

        return stream;
    }

    namespace JsonToken
    {
        bool End::operator==(const End&) const
        {
            return true;
        }

        bool End::operator!=(const End&) const
        {
            return false;
        }

        bool Error::operator==(const Error&) const
        {
            return true;
        }

        bool Error::operator!=(const Error&) const
        {
            return false;
        }

        bool Colon::operator==(const Colon&) const
        {
            return true;
        }

        bool Colon::operator!=(const Colon&) const
        {
            return false;
        }

        bool Comma::operator==(const Comma&) const
        {
            return true;
        }

        bool Comma::operator!=(const Comma&) const
        {
            return false;
        }

        bool Dot::operator==(const Dot&) const
        {
            return true;
        }

        bool Dot::operator!=(const Dot&) const
        {
            return false;
        }

        bool Null::operator==(const Null&) const
        {
            return true;
        }

        bool Null::operator!=(const Null&) const
        {
            return false;
        }

        LeftBrace::LeftBrace(std::size_t index)
            : index(index)
        {}

        bool LeftBrace::operator==(const LeftBrace& other) const
        {
            return index == other.index;
        }

        bool LeftBrace::operator!=(const LeftBrace& other) const
        {
            return index != other.index;
        }

        std::size_t LeftBrace::Index() const
        {
            return index;
        }

        RightBrace::RightBrace(std::size_t index)
            : index(index)
        {}

        bool RightBrace::operator==(const RightBrace& other) const
        {
            return index == other.index;
        }

        bool RightBrace::operator!=(const RightBrace& other) const
        {
            return index != other.index;
        }

        std::size_t RightBrace::Index() const
        {
            return index;
        }

        LeftBracket::LeftBracket(std::size_t index)
            : index(index)
        {}

        bool LeftBracket::operator==(const LeftBracket& other) const
        {
            return index == other.index;
        }

        bool LeftBracket::operator!=(const LeftBracket& other) const
        {
            return index != other.index;
        }

        std::size_t LeftBracket::Index() const
        {
            return index;
        }

        RightBracket::RightBracket(std::size_t index)
            : index(index)
        {}

        bool RightBracket::operator==(const RightBracket& other) const
        {
            return index == other.index;
        }

        bool RightBracket::operator!=(const RightBracket& other) const
        {
            return index != other.index;
        }

        std::size_t RightBracket::Index() const
        {
            return index;
        }

        String::String(infra::BoundedConstString value)
            : value(value)
        {}

        bool String::operator==(const String& other) const
        {
            return value == other.value;
        }

        bool String::operator!=(const String& other) const
        {
            return value != other.value;
        }

        JsonString String::Value() const
        {
            return value;
        }

        BoundedConstString String::RawValue() const
        {
            return value.Raw();
        }

        Boolean::Boolean(bool value)
            : value(value)
        {}

        bool Boolean::operator==(const Boolean& other) const
        {
            return value == other.value;
        }

        bool Boolean::operator!=(const Boolean& other) const
        {
            return value != other.value;
        }

        bool Boolean::Value() const
        {
            return value;
        }
    }

    JsonTokenizer::JsonTokenizer(infra::BoundedConstString objectString)
        : objectString(objectString)
    {}

    JsonToken::Token JsonTokenizer::Token()
    {
        SkipWhitespace();

        if (parseIndex == objectString.size())
            return JsonToken::End();
        else
        {
            if (objectString[parseIndex] == '"')
                return TryCreateStringToken();
            else if (objectString[parseIndex] == ':')
            {
                ++parseIndex;
                return JsonToken::Colon();
            }
            else if (objectString[parseIndex] == ',')
            {
                ++parseIndex;
                return JsonToken::Comma();
            }
            else if (objectString[parseIndex] == '.')
            {
                ++parseIndex;
                return JsonToken::Dot();
            }
            else if (objectString[parseIndex] == '{')
                return JsonToken::LeftBrace(parseIndex++);
            else if (objectString[parseIndex] == '}')
                return JsonToken::RightBrace(parseIndex++);
            else if (objectString[parseIndex] == '[')
                return JsonToken::LeftBracket(parseIndex++);
            else if (objectString[parseIndex] == ']')
                return JsonToken::RightBracket(parseIndex++);
            else if (std::isdigit(static_cast<unsigned char>(objectString[parseIndex])) != 0 || objectString[parseIndex] == '-')
                return TryCreateIntegerOrFloatToken();
            else if (std::isalpha(static_cast<unsigned char>(objectString[parseIndex])) != 0)
                return TryCreateIdentifierToken();
            else
                return JsonToken::Error();
        }
    }

    bool JsonTokenizer::operator==(const JsonTokenizer& other) const
    {
        return parseIndex == other.parseIndex;
    }

    bool JsonTokenizer::operator!=(const JsonTokenizer& other) const
    {
        return !(*this == other);
    }

    void JsonTokenizer::SkipWhitespace()
    {
        while (parseIndex != objectString.size() && std::isspace(static_cast<unsigned char>(objectString[parseIndex])))
            ++parseIndex;
    }

    JsonToken::Token JsonTokenizer::TryCreateStringToken()
    {
        ++parseIndex;
        std::size_t tokenStart = parseIndex;

        if (parseIndex == objectString.size())
            return JsonToken::Error();

        bool escape = false;
        while (escape || objectString[parseIndex] != '"')
        {
            escape = !escape && objectString[parseIndex] == '\\';
            ++parseIndex;

            if (parseIndex == objectString.size())
                return JsonToken::Error();
        }

        ++parseIndex;

        return JsonToken::String(objectString.substr(tokenStart, parseIndex - tokenStart - 1));
    }

    JsonToken::Token JsonTokenizer::TryCreateIntegerOrFloatToken()
    {
        std::size_t tokenStart = parseIndex;
        bool sign = false;

        if (parseIndex != objectString.size() && objectString[parseIndex] == '-')
        {
            sign = true;
            ++parseIndex;
        }

        while (parseIndex != objectString.size() && std::isdigit(static_cast<unsigned char>(objectString[parseIndex])))
            ++parseIndex;

        auto value = ToInt<uint64_t>(objectString.substr(tokenStart, parseIndex - tokenStart), sign);

        if (parseIndex == objectString.size() || objectString[parseIndex] != '.')
            return JsonBiggerInt(value, sign);

        ++parseIndex;

        return TryCreateFloatToken(value, sign);
    }

    JsonToken::Token JsonTokenizer::TryCreateIdentifierToken()
    {
        std::size_t tokenStart = parseIndex;

        while (parseIndex != objectString.size() && std::isalpha(static_cast<unsigned char>(objectString[parseIndex])))
            ++parseIndex;

        infra::BoundedConstString identifier = objectString.substr(tokenStart, parseIndex - tokenStart);
        if (identifier == "true")
            return JsonToken::Boolean(true);
        else if (identifier == "false")
            return JsonToken::Boolean(false);
        else if (identifier == "null")
            return JsonToken::Null();
        else
            return JsonToken::Error();
    }

    JsonToken::Token JsonTokenizer::TryCreateFloatToken(uint64_t integer, bool sign)
    {
        if (parseIndex != objectString.size() && objectString[parseIndex] == '-')
            return JsonToken::Error();

        auto tokenStart = parseIndex;

        while (parseIndex != objectString.size() && std::isdigit(static_cast<unsigned char>(objectString[parseIndex])))
            ++parseIndex;

        infra::BoundedConstString fractional = objectString.substr(tokenStart, parseIndex - tokenStart);
        fractional.shrink(nanoValueWidth);

        auto fractionalValue = ToInt<uint32_t>(fractional, false);

        if (fractional.size() < 9)
            for (std::size_t count = fractional.size(); count != nanoValueWidth; ++count)
                fractionalValue = fractionalValue * 10;

        return JsonFloat(integer, fractionalValue, sign);
    }

    JsonObject::JsonObject(infra::BoundedConstString objectString)
        : objectString(objectString)
    {}

    infra::BoundedConstString JsonObject::ObjectString() const
    {
        return objectString;
    }

    JsonObjectIterator JsonObject::begin()
    {
        return JsonObjectIterator(*this);
    }

    JsonObjectIterator JsonObject::end()
    {
        return JsonObjectIterator();
    }

    bool JsonObject::HasKey(infra::BoundedConstString key)
    {
        return std::any_of(begin(), end(), [key](const auto& keyValue)
            {
                return keyValue.key == key;
            });
    }

    JsonString JsonObject::GetString(infra::BoundedConstString key)
    {
        return GetValue<JsonString>(key);
    }

    JsonFloat JsonObject::GetFloat(infra::BoundedConstString key)
    {
        return GetValue<JsonFloat>(key);
    }

    bool JsonObject::GetBoolean(infra::BoundedConstString key)
    {
        return GetValue<bool>(key);
    }

    int32_t JsonObject::GetInteger(infra::BoundedConstString key)
    {
        return GetValue<int32_t>(key);
    }

    JsonObject JsonObject::GetObject(infra::BoundedConstString key)
    {
        return GetValue<JsonObject>(key);
    }

    JsonArray JsonObject::GetArray(infra::BoundedConstString key)
    {
        return GetValue<JsonArray>(key);
    }

    JsonValue JsonObject::GetValue(infra::BoundedConstString key)
    {
        for (auto& keyValue : *this)
        {
            if (keyValue.key == key)
                return keyValue.value;
        }

        SetError();
        return JsonValue();
    }

    std::optional<JsonString> JsonObject::GetOptionalString(infra::BoundedConstString key)
    {
        return GetOptionalValue<JsonString>(key);
    }

    std::optional<JsonFloat> JsonObject::GetOptionalFloat(infra::BoundedConstString key)
    {
        return GetOptionalValue<JsonFloat>(key);
    }

    std::optional<bool> JsonObject::GetOptionalBoolean(infra::BoundedConstString key)
    {
        return GetOptionalValue<bool>(key);
    }

    std::optional<int32_t> JsonObject::GetOptionalInteger(infra::BoundedConstString key)
    {
        return GetOptionalValue<int32_t>(key);
    }

    std::optional<JsonObject> JsonObject::GetOptionalObject(infra::BoundedConstString key)
    {
        return GetOptionalValue<JsonObject>(key);
    }

    std::optional<JsonArray> JsonObject::GetOptionalArray(infra::BoundedConstString key)
    {
        return GetOptionalValue<JsonArray>(key);
    }

    bool JsonObject::operator==(const JsonObject& other) const
    {
        JsonObject left(*this);
        JsonObject right(other);

        bool result = Equal(left.begin(), left.end(), right.begin(), right.end());
        return result && !left.Error() && !right.Error();
    }

    bool JsonObject::operator!=(const JsonObject& other) const
    {
        return !(*this == other);
    }

    void JsonObject::SetError()
    {
        error = true;
    }

    bool JsonObject::Error() const
    {
        return error;
    }

    template<class T>
    T JsonObject::GetValue(infra::BoundedConstString key)
    {
        for (auto& keyValue : *this)
        {
            if (keyValue.key == key && keyValue.value.Is<T>())
                return keyValue.value.Get<T>();
        }

        SetError();
        return T();
    }

    template<class T>
    std::optional<T> JsonObject::GetOptionalValue(infra::BoundedConstString key)
    {
        for (auto& keyValue : *this)
        {
            if (keyValue.key == key && keyValue.value.Is<T>())
                return std::make_optional(keyValue.value.Get<T>());
        }

        return std::nullopt;
    }

    JsonArray::JsonArray(infra::BoundedConstString objectString)
        : objectString(objectString)
    {}

    infra::BoundedConstString JsonArray::ObjectString() const
    {
        return objectString;
    }

    void JsonArray::SetError()
    {
        error = true;
    }

    bool JsonArray::Error() const
    {
        return error;
    }

    JsonArrayIterator JsonArray::begin()
    {
        return JsonArrayIterator(*this);
    }

    JsonArrayIterator JsonArray::end()
    {
        return JsonArrayIterator();
    }

    bool JsonArray::operator==(const JsonArray& other) const
    {
        JsonArray left(*this);
        JsonArray right(other);

        bool result = Equal(left.begin(), left.end(), right.begin(), right.end());
        return result && !left.Error() && !right.Error();
    }

    bool JsonArray::operator!=(const JsonArray& other) const
    {
        return !(*this == other);
    }

    bool JsonKeyValue::operator==(const JsonKeyValue& other) const
    {
        return key == other.key && value == other.value;
    }

    bool JsonKeyValue::operator!=(const JsonKeyValue& other) const
    {
        return !(*this == other);
    }

    JsonIterator::JsonIterator(infra::BoundedConstString objectString)
        : objectString(objectString)
        , tokenizer(objectString)
    {}

    std::optional<JsonValue> JsonIterator::ConvertValue(JsonToken::Token token)
    {
        if (token.Is<JsonToken::String>())
            return std::make_optional(JsonValue(token.Get<JsonToken::String>().Value()));
        else if (token.Is<JsonBiggerInt>())
            return ReadInteger(token);
        else if (token.Is<JsonFloat>())
            return std::make_optional(JsonValue(token.Get<JsonFloat>()));
        else if (token.Is<JsonToken::Boolean>())
            return std::make_optional(JsonValue(token.Get<JsonToken::Boolean>().Value()));
        else if (token.Is<JsonToken::LeftBrace>())
            return ReadObjectValue(token);
        else if (token.Is<JsonToken::LeftBracket>())
            return ReadArrayValue(token);
        else if (token.Is<JsonToken::Null>())
            return std::make_optional(JsonValue(JsonObject()));
        else
            return std::nullopt;
    }

    std::optional<JsonValue> JsonIterator::ReadInteger(const JsonToken::Token& token)
    {
        if ((!token.Get<JsonBiggerInt>().Negative() && token.Get<JsonBiggerInt>().Value() <= std::numeric_limits<int32_t>::max()) || (token.Get<JsonBiggerInt>().Negative() && token.Get<JsonBiggerInt>().Value() <= static_cast<uint64_t>(-static_cast<int64_t>(std::numeric_limits<int32_t>::min()))))
            return std::make_optional(JsonValue(static_cast<int32_t>(token.Get<JsonBiggerInt>().Value() * (token.Get<JsonBiggerInt>().Negative() ? -1 : 1))));
        else
            return std::make_optional(JsonValue(token.Get<JsonBiggerInt>()));
    }

    std::optional<JsonValue> JsonIterator::ReadObjectValue(const JsonToken::Token& token)
    {
        std::optional<JsonToken::RightBrace> objectEnd = SearchObjectEnd();

        if (objectEnd)
            return std::make_optional(JsonValue(JsonObject(objectString.substr(token.Get<JsonToken::LeftBrace>().Index(), objectEnd->Index() + 1 - token.Get<JsonToken::LeftBrace>().Index()))));
        else
            return std::nullopt;
    }

    std::optional<JsonValue> JsonIterator::ReadArrayValue(const JsonToken::Token& token)
    {
        std::optional<JsonToken::RightBracket> arrayEnd = SearchArrayEnd();

        if (arrayEnd)
            return std::make_optional(JsonValue(JsonArray(objectString.substr(token.Get<JsonToken::LeftBracket>().Index(), arrayEnd->Index() + 1 - token.Get<JsonToken::LeftBracket>().Index()))));
        else
            return std::nullopt;
    }

    std::optional<JsonToken::RightBrace> JsonIterator::SearchObjectEnd()
    {
        std::size_t nested = 0;
        JsonToken::Token token = tokenizer.Token();
        while ((nested != 0 || !token.Is<JsonToken::RightBrace>()) && !token.Is<JsonToken::End>() && !token.Is<JsonToken::Error>())
        {
            if (token.Is<JsonToken::LeftBrace>())
                --nested;
            if (token.Is<JsonToken::RightBrace>())
                ++nested;

            token = tokenizer.Token();
        }

        if (token.Is<JsonToken::RightBrace>())
            return std::make_optional(token.Get<JsonToken::RightBrace>());
        return std::nullopt;
    }

    std::optional<JsonToken::RightBracket> JsonIterator::SearchArrayEnd()
    {
        std::size_t nested = 0;
        JsonToken::Token token = tokenizer.Token();
        while ((nested != 0 || !token.Is<JsonToken::RightBracket>()) && !token.Is<JsonToken::End>() && !token.Is<JsonToken::Error>())
        {
            if (token.Is<JsonToken::LeftBracket>())
                --nested;
            if (token.Is<JsonToken::RightBracket>())
                ++nested;

            token = tokenizer.Token();
        }

        if (token.Is<JsonToken::RightBracket>())
            return std::make_optional(token.Get<JsonToken::RightBracket>());
        else
            return std::nullopt;
    }

    JsonObjectIterator::JsonObjectIterator()
        : JsonIterator("")
    {}

    JsonObjectIterator::JsonObjectIterator(JsonObject& object)
        : JsonIterator(object.ObjectString())
        , object(&object)
        , state(readObjectStart)
    {
        ++*this;
    }

    bool JsonObjectIterator::operator==(const JsonObjectIterator& other) const
    {
        return state == other.state && (state == end || tokenizer == other.tokenizer);
    }

    bool JsonObjectIterator::operator!=(const JsonObjectIterator& other) const
    {
        return !(*this == other);
    }

    JsonKeyValue& JsonObjectIterator::operator*()
    {
        assert(state != end);
        return keyValue;
    }

    const JsonKeyValue& JsonObjectIterator::operator*() const
    {
        assert(state != end);
        return keyValue;
    }

    JsonKeyValue* JsonObjectIterator::operator->()
    {
        assert(state != end);
        return &keyValue;
    }

    const JsonKeyValue* JsonObjectIterator::operator->() const
    {
        assert(state != end);
        return &keyValue;
    }

    JsonObjectIterator& JsonObjectIterator::operator++()
    {
        do
        {
            ParseNextToken();
        } while (state != readCommaOrObjectEnd && state != end);

        return *this;
    }

    void JsonObjectIterator::ParseNextToken()
    {
        JsonToken::Token token = tokenizer.Token();
        switch (state)
        {
            case readObjectStart:
                ReadObjectStart(token);
                break;
            case readKeyOrEnd:
                ReadKeyOrEnd(token);
                break;
            case readKey:
                ReadKey(token);
                break;
            case readColon:
                ReadColon(token);
                break;
            case readValue:
                ReadValue(token);
                break;
            case readCommaOrObjectEnd:
                ReadCommaOrObjectEnd(token);
                break;
            case end:
                std::abort();
        }
    }

    void JsonObjectIterator::ReadObjectStart(JsonToken::Token token)
    {
        state = readKeyOrEnd;
        if (!token.Is<JsonToken::LeftBrace>())
            SetError();
    }

    void JsonObjectIterator::ReadKeyOrEnd(JsonToken::Token token)
    {
        state = readColon;
        if (token.Is<JsonToken::RightBrace>())
            state = end;
        else if (token.Is<JsonToken::String>())
            keyValue.key = token.Get<JsonToken::String>().Value();
        else
            SetError();
    }

    void JsonObjectIterator::ReadKey(JsonToken::Token token)
    {
        state = readColon;
        if (token.Is<JsonToken::String>())
            keyValue.key = token.Get<JsonToken::String>().Value();
        else
            SetError();
    }

    void JsonObjectIterator::ReadColon(JsonToken::Token token)
    {
        state = readValue;
        if (!token.Is<JsonToken::Colon>())
            SetError();
    }

    void JsonObjectIterator::ReadValue(JsonToken::Token token)
    {
        state = readCommaOrObjectEnd;
        std::optional<JsonValue> readValue = ConvertValue(token);
        if (readValue)
            keyValue.value = *readValue;
        else
            SetError();
    }

    void JsonObjectIterator::ReadCommaOrObjectEnd(JsonToken::Token token)
    {
        if (token.Is<JsonToken::Comma>())
            state = readKey;
        else if (token.Is<JsonToken::RightBrace>())
            state = end;
        else
            SetError();
    }

    void JsonObjectIterator::SetError()
    {
        object->SetError();
        state = end;
    }

    JsonObjectIterator JsonObjectIterator::operator++(int)
    {
        JsonObjectIterator result(*this);
        ++*this;
        return result;
    }

    JsonArrayIterator::JsonArrayIterator()
        : JsonIterator("")
    {}

    JsonArrayIterator::JsonArrayIterator(JsonArray& jsonArray)
        : JsonIterator(jsonArray.ObjectString())
        , jsonArray(&jsonArray)
        , state(readArrayStart)
    {
        ++*this;
    }

    bool JsonArrayIterator::operator==(const JsonArrayIterator& other) const
    {
        return state == other.state && (state == end || tokenizer == other.tokenizer);
    }

    bool JsonArrayIterator::operator!=(const JsonArrayIterator& other) const
    {
        return !(*this == other);
    }

    JsonValue& JsonArrayIterator::operator*()
    {
        assert(state != end);
        return value;
    }

    const JsonValue& JsonArrayIterator::operator*() const
    {
        assert(state != end);
        return value;
    }

    JsonValue* JsonArrayIterator::operator->()
    {
        assert(state != end);
        return &value;
    }

    const JsonValue* JsonArrayIterator::operator->() const
    {
        assert(state != end);
        return &value;
    }

    JsonArrayIterator& JsonArrayIterator::operator++()
    {
        do
        {
            JsonToken::Token token = tokenizer.Token();
            switch (state)
            {
                case readArrayStart:
                    state = readValueOrEnd;
                    if (!token.Is<JsonToken::LeftBracket>())
                        SetError();
                    break;
                case readValueOrEnd:
                    state = readCommaOrArrayEnd;
                    if (token.Is<JsonToken::RightBracket>())
                        state = end;
                    else
                        ReadValue(token);
                    break;
                case readValue:
                    state = readCommaOrArrayEnd;
                    ReadValue(token);
                    break;
                case readCommaOrArrayEnd:
                    if (token.Is<JsonToken::Comma>())
                        state = readValue;
                    else if (token.Is<JsonToken::RightBracket>())
                        state = end;
                    else
                        SetError();
                    break;
                case end:
                    std::abort();
            }
        } while (state != readCommaOrArrayEnd && state != end);

        return *this;
    }

    JsonArrayIterator JsonArrayIterator::operator++(int)
    {
        JsonArrayIterator result(*this);
        ++*this;
        return result;
    }

    void JsonArrayIterator::ReadValue(JsonToken::Token token)
    {
        std::optional<JsonValue> readValue = ConvertValue(token);
        if (readValue)
            value = *readValue;
        else
            SetError();
    }

    void JsonArrayIterator::SetError()
    {
        jsonArray->SetError();
        state = end;
    }

    infra::detail::DoublePair<JsonValueArrayIterator<bool>> JsonBooleanArray(JsonArray& array)
    {
        return infra::detail::DoublePair<JsonValueArrayIterator<bool>>(
            JsonValueArrayIterator<bool>(array.begin(), array.end()),
            JsonValueArrayIterator<bool>(array.end(), array.end()));
    }

    infra::detail::DoublePair<JsonValueArrayIterator<int32_t>> JsonIntegerArray(JsonArray& array)
    {
        return infra::detail::DoublePair<JsonValueArrayIterator<int32_t>>(
            JsonValueArrayIterator<int32_t>(array.begin(), array.end()),
            JsonValueArrayIterator<int32_t>(array.end(), array.end()));
    }

    infra::detail::DoublePair<JsonValueArrayIterator<JsonString>> JsonStringArray(JsonArray& array)
    {
        return infra::detail::DoublePair<JsonValueArrayIterator<JsonString>>(
            JsonValueArrayIterator<JsonString>(array.begin(), array.end()),
            JsonValueArrayIterator<JsonString>(array.end(), array.end()));
    }

    infra::detail::DoublePair<JsonValueArrayIterator<JsonObject>> JsonObjectArray(JsonArray& array)
    {
        return infra::detail::DoublePair<JsonValueArrayIterator<JsonObject>>(
            JsonValueArrayIterator<JsonObject>(array.begin(), array.end()),
            JsonValueArrayIterator<JsonObject>(array.end(), array.end()));
    }

    infra::detail::DoublePair<JsonValueArrayIterator<JsonArray>> JsonArrayArray(JsonArray& array)
    {
        return infra::detail::DoublePair<JsonValueArrayIterator<JsonArray>>(
            JsonValueArrayIterator<JsonArray>(array.begin(), array.end()),
            JsonValueArrayIterator<JsonArray>(array.end(), array.end()));
    }

    void CopyToken(infra::JsonToken::Token token, infra::TextOutputStream& stream)
    {
        class CopyVisitor
            : public infra::StaticVisitor<void>
        {
        public:
            explicit CopyVisitor(infra::TextOutputStream& stream)
                : stream(stream)
            {}

            void operator()(infra::JsonToken::End) const
            {
                std::abort();
            }

            void operator()(infra::JsonToken::Error) const
            {
                std::abort();
            }

            void operator()(infra::JsonToken::Colon)
            {
                stream << ':';
            }

            void operator()(infra::JsonToken::Comma)
            {
                stream << ',';
            }

            void operator()(infra::JsonToken::Dot)
            {
                stream << '.';
            }

            void operator()(infra::JsonToken::Null)
            {
                stream << "null";
            }

            void operator()(infra::JsonToken::LeftBrace)
            {
                stream << '{';
            }

            void operator()(infra::JsonToken::RightBrace)
            {
                stream << '}';
            }

            void operator()(infra::JsonToken::LeftBracket)
            {
                stream << '[';
            }

            void operator()(infra::JsonToken::RightBracket)
            {
                stream << ']';
            }

            void operator()(infra::JsonToken::String token)
            {
                stream << '"' << token.RawValue() << '"';
            }

            void operator()(infra::JsonBiggerInt token)
            {
                if (token.Negative())
                    stream << '-';

                stream << token.Value();
            }

            void operator()(infra::JsonFloat token)
            {
                if (token.Negative())
                    stream << '-';

                auto fractional = token.NanoFractionalValue();
                auto width = nanoValueWidth;
                while (width != 1 && (fractional % 10 == 0))
                {
                    fractional = fractional / 10;
                    --width;
                }

                stream << token.IntValue() << "." << infra::Width(width, '0') << fractional;
            }

            void operator()(infra::JsonToken::Boolean token)
            {
                if (token.Value())
                    stream << "true";
                else
                    stream << "false";
            }

        private:
            infra::TextOutputStream& stream;
        };

        CopyVisitor copyVisitor(stream);
        infra::ApplyVisitor(copyVisitor, token);
    }

    void CleanJsonContents(infra::BoundedString& contents)
    {
        infra::JsonTokenizer tokenizer(contents);
        infra::BoundedString newContents(contents);
        newContents.clear();
        infra::StringOutputStream stream(newContents);

        for (infra::JsonToken::Token token = tokenizer.Token(); !token.Is<infra::JsonToken::End>() && !token.Is<infra::JsonToken::Error>(); token = tokenizer.Token())
            CopyToken(token, stream);

        contents = newContents;
    }

    bool ValidJsonObject(infra::BoundedConstString contents)
    {
        JsonObject object(contents);
        for ([[maybe_unused]] const auto& i : object)
        {}

        return !object.Error();
    }
}
