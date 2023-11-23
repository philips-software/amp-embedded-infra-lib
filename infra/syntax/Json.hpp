#ifndef INFRA_JSON_HPP
#define INFRA_JSON_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/ReverseRange.hpp"
#include "infra/util/Variant.hpp"

#ifdef EMIL_HOST_BUILD
#include <string>
#endif

namespace infra
{
    class JsonStringIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        JsonStringIterator(infra::BoundedConstString::const_iterator position, infra::BoundedConstString::const_iterator end);

        char operator*() const;

        JsonStringIterator& operator++();
        JsonStringIterator operator++(int);

        bool operator==(const JsonStringIterator& other) const;
        bool operator!=(const JsonStringIterator& other) const;

    private:
        infra::BoundedConstString::const_iterator position;
        infra::BoundedConstString::const_iterator end;
    };

    class JsonString
    {
    public:
        JsonString() = default;
        JsonString(infra::BoundedConstString source);
        JsonString(const char* source);

        bool operator==(const JsonString& other) const;
        bool operator!=(const JsonString& other) const;

        bool operator==(infra::BoundedConstString other) const;
        bool operator!=(infra::BoundedConstString other) const;
        friend bool operator==(infra::BoundedConstString x, JsonString y);
        friend bool operator!=(infra::BoundedConstString x, JsonString y);
        bool operator==(const char* other) const;
        bool operator!=(const char* other) const;
        friend bool operator==(const char* x, JsonString y);
        friend bool operator!=(const char* x, JsonString y);

        bool empty() const;
        std::size_t size() const;
        JsonStringIterator begin() const;
        JsonStringIterator end() const;

        infra::BoundedConstString Raw() const;

        void ToString(infra::BoundedString& result) const;
        void AppendTo(infra::BoundedString& result) const;
#ifdef EMIL_HOST_BUILD
        std::string ToStdString() const;
#endif

        friend infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, JsonString value);

    private:
        infra::BoundedConstString source;
    };

    class JsonFloat
    {
    public:
        JsonFloat() = default;
        JsonFloat(uint64_t intValue, uint32_t nanoFractionalValue, bool negative);

        bool operator==(const JsonFloat& other) const;
        bool operator!=(const JsonFloat& other) const;

        uint64_t IntValue() const;
        uint32_t NanoFractionalValue() const;
        bool Negative() const;

    private:
        uint64_t intValue = 0;
        uint32_t nanoFractionalValue = 0;
        bool negative = false;
    };

    class JsonBiggerInt
    {
    public:
        JsonBiggerInt() = default;
        JsonBiggerInt(uint64_t value, bool negative);

        bool operator==(const JsonBiggerInt& other) const;
        bool operator!=(const JsonBiggerInt& other) const;

        uint64_t Value() const;
        bool Negative() const;

    private:
        uint64_t value = 0;
        bool negative = false;
    };

    namespace JsonToken
    {
        class End
        {
        public:
            bool operator==(const End& other) const;
            bool operator!=(const End& other) const;
        };

        class Error
        {
        public:
            bool operator==(const Error& other) const;
            bool operator!=(const Error& other) const;
        };

        class Colon
        {
        public:
            bool operator==(const Colon& other) const;
            bool operator!=(const Colon& other) const;
        };

        class Comma
        {
        public:
            bool operator==(const Comma& other) const;
            bool operator!=(const Comma& other) const;
        };

        class Dot
        {
        public:
            bool operator==(const Dot& other) const;
            bool operator!=(const Dot& other) const;
        };

        class Null
        {
        public:
            bool operator==(const Null& other) const;
            bool operator!=(const Null& other) const;
        };

        class LeftBrace
        {
        public:
            explicit LeftBrace(std::size_t index);

            bool operator==(const LeftBrace& other) const;
            bool operator!=(const LeftBrace& other) const;

            std::size_t Index() const;

        private:
            std::size_t index;
        };

        class RightBrace
        {
        public:
            explicit RightBrace(std::size_t index);

            bool operator==(const RightBrace& other) const;
            bool operator!=(const RightBrace& other) const;

            std::size_t Index() const;

        private:
            std::size_t index;
        };

        class LeftBracket
        {
        public:
            explicit LeftBracket(std::size_t index);

            bool operator==(const LeftBracket& other) const;
            bool operator!=(const LeftBracket& other) const;

            std::size_t Index() const;

        private:
            std::size_t index;
        };

        class RightBracket
        {
        public:
            explicit RightBracket(std::size_t index);

            bool operator==(const RightBracket& other) const;
            bool operator!=(const RightBracket& other) const;

            std::size_t Index() const;

        private:
            std::size_t index;
        };

        class String
        {
        public:
            explicit String(infra::BoundedConstString value);

            bool operator==(const String& other) const;
            bool operator!=(const String& other) const;

            JsonString Value() const;
            BoundedConstString RawValue() const;

        private:
            JsonString value;
        };

        class Boolean
        {
        public:
            explicit Boolean(bool value);

            bool operator==(const Boolean& other) const;
            bool operator!=(const Boolean& other) const;

            bool Value() const;

        private:
            bool value;
        };

        using Token = infra::Variant<End, Error, Colon, Comma, Dot, Null, LeftBrace, RightBrace, LeftBracket, RightBracket, String, JsonBiggerInt, JsonFloat, Boolean>;
    }

    class JsonTokenizer
    {
    public:
        explicit JsonTokenizer(infra::BoundedConstString objectString);

        JsonToken::Token Token();

        bool operator==(const JsonTokenizer& other) const;
        bool operator!=(const JsonTokenizer& other) const;

    private:
        void SkipWhitespace();
        JsonToken::Token TryCreateStringToken();
        JsonToken::Token TryCreateIntegerOrFloatToken();
        JsonToken::Token TryCreateIdentifierToken();
        JsonToken::Token TryCreateFloatToken(uint64_t integer, bool sign);

    private:
        infra::BoundedConstString objectString;
        std::size_t parseIndex = 0;
    };

    class JsonObjectIterator;
    class JsonArrayIterator;
    class JsonObject;
    class JsonArray;

    using JsonValue = infra::Variant<bool, int32_t, JsonBiggerInt, JsonString, JsonFloat, JsonObject, JsonArray>;

    class JsonObject
    {
    public:
        JsonObject() = default;
        explicit JsonObject(infra::BoundedConstString objectString);

        infra::BoundedConstString ObjectString() const;

        JsonObjectIterator begin();
        JsonObjectIterator end();

        bool HasKey(infra::BoundedConstString key);

        JsonString GetString(infra::BoundedConstString key);
        JsonFloat GetFloat(infra::BoundedConstString key);
        bool GetBoolean(infra::BoundedConstString key);
        int32_t GetInteger(infra::BoundedConstString key);
        JsonObject GetObject(infra::BoundedConstString key);
        JsonArray GetArray(infra::BoundedConstString key);
        JsonValue GetValue(infra::BoundedConstString key);

        infra::Optional<JsonString> GetOptionalString(infra::BoundedConstString key);
        infra::Optional<JsonFloat> GetOptionalFloat(infra::BoundedConstString key);
        infra::Optional<bool> GetOptionalBoolean(infra::BoundedConstString key);
        infra::Optional<int32_t> GetOptionalInteger(infra::BoundedConstString key);
        infra::Optional<JsonObject> GetOptionalObject(infra::BoundedConstString key);
        infra::Optional<JsonArray> GetOptionalArray(infra::BoundedConstString key);

        bool operator==(const JsonObject& other) const;
        bool operator!=(const JsonObject& other) const;

    public:
        void SetError();
        bool Error() const;

    private:
        template<class T>
        T GetValue(infra::BoundedConstString key);
        template<class T>
        infra::Optional<T> GetOptionalValue(infra::BoundedConstString key);

    private:
        infra::BoundedConstString objectString;
        bool error = false;
    };

    class JsonArray
    {
    public:
        JsonArray() = default;
        explicit JsonArray(infra::BoundedConstString objectString);

        infra::BoundedConstString ObjectString() const;

        JsonArrayIterator begin();
        JsonArrayIterator end();

        bool operator==(const JsonArray& other) const;
        bool operator!=(const JsonArray& other) const;

    public:
        void SetError();
        bool Error() const;

    private:
        infra::BoundedConstString objectString;
        bool error = false;
    };

    struct JsonKeyValue
    {
        bool operator==(const JsonKeyValue& other) const;
        bool operator!=(const JsonKeyValue& other) const;

        JsonString key;
        JsonValue value;
    };

    class JsonIterator
    {
    protected:
        explicit JsonIterator(infra::BoundedConstString objectString);

        infra::Optional<JsonValue> ConvertValue(JsonToken::Token token);

    private:
        infra::Optional<JsonValue> ReadInteger(const JsonToken::Token& token);
        infra::Optional<JsonValue> ReadObjectValue(const JsonToken::Token& token);
        infra::Optional<JsonValue> ReadArrayValue(const JsonToken::Token& token);
        infra::Optional<JsonToken::RightBrace> SearchObjectEnd();
        infra::Optional<JsonToken::RightBracket> SearchArrayEnd();

    protected:
        infra::BoundedConstString objectString;
        JsonTokenizer tokenizer;
    };

    class JsonObjectIterator
        : private JsonIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = JsonKeyValue;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

    private:
        friend class JsonObject;

    public:
        JsonObjectIterator();

    private:
        explicit JsonObjectIterator(JsonObject& object);

    public:
        bool operator==(const JsonObjectIterator& other) const;
        bool operator!=(const JsonObjectIterator& other) const;

        JsonKeyValue& operator*();
        const JsonKeyValue& operator*() const;
        JsonKeyValue* operator->();
        const JsonKeyValue* operator->() const;

        JsonObjectIterator& operator++();
        JsonObjectIterator operator++(int);

    private:
        void ParseNextToken();
        void SetError();

        void ReadObjectStart(JsonToken::Token token);
        void ReadKeyOrEnd(JsonToken::Token token);
        void ReadKey(JsonToken::Token token);
        void ReadColon(JsonToken::Token token);
        void ReadValue(JsonToken::Token token);
        void ReadCommaOrObjectEnd(JsonToken::Token token);

    private:
        JsonObject* object = nullptr;
        JsonKeyValue keyValue;

        enum
        {
            readObjectStart,
            readKeyOrEnd,
            readKey,
            readColon,
            readValue,
            readCommaOrObjectEnd,
            end
        } state = end;
    };

    class JsonArrayIterator
        : private JsonIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = JsonValue;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

    private:
        friend class JsonArray;

        template<class T>
        friend class JsonValueArrayIterator;

    public:
        JsonArrayIterator();

    private:
        explicit JsonArrayIterator(JsonArray& jsonArray);

    public:
        bool operator==(const JsonArrayIterator& other) const;
        bool operator!=(const JsonArrayIterator& other) const;

        JsonValue& operator*();
        const JsonValue& operator*() const;
        JsonValue* operator->();
        const JsonValue* operator->() const;

        JsonArrayIterator& operator++();
        JsonArrayIterator operator++(int);

    private:
        void ReadValue(JsonToken::Token token);
        void SetError();

    private:
        JsonArray* jsonArray = nullptr;
        JsonValue value;

        enum
        {
            readArrayStart,
            readValueOrEnd,
            readValue,
            readCommaOrArrayEnd,
            end
        } state = end;
    };

    template<class T>
    class JsonValueArrayIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        JsonValueArrayIterator() = default;
        JsonValueArrayIterator(const JsonArrayIterator& arrayIterator, const JsonArrayIterator& arrayEndIterator);

    public:
        bool operator==(const JsonValueArrayIterator& other) const;
        bool operator!=(const JsonValueArrayIterator& other) const;

        T operator*() const;
        const T* operator->() const;

        JsonValueArrayIterator& operator++();
        JsonValueArrayIterator operator++(int);

    private:
        JsonArrayIterator arrayIterator;
        JsonArrayIterator arrayEndIterator;
    };

    infra::detail::DoublePair<JsonValueArrayIterator<bool>> JsonBooleanArray(JsonArray& array);
    infra::detail::DoublePair<JsonValueArrayIterator<int32_t>> JsonIntegerArray(JsonArray& array);
    infra::detail::DoublePair<JsonValueArrayIterator<JsonString>> JsonStringArray(JsonArray& array);
    infra::detail::DoublePair<JsonValueArrayIterator<JsonObject>> JsonObjectArray(JsonArray& array);
    infra::detail::DoublePair<JsonValueArrayIterator<JsonArray>> JsonArrayArray(JsonArray& array);
    void CleanJsonContents(infra::BoundedString& contents);
    bool ValidJsonObject(infra::BoundedConstString contents);

    ////    Implementation    ////

    template<class T>
    JsonValueArrayIterator<T>::JsonValueArrayIterator(const JsonArrayIterator& arrayIterator, const JsonArrayIterator& arrayEndIterator)
        : arrayIterator(arrayIterator)
        , arrayEndIterator(arrayEndIterator)
    {
        if (this->arrayIterator != arrayEndIterator && !this->arrayIterator->template Is<T>())
            this->arrayIterator.SetError();
    }

    template<class T>
    bool JsonValueArrayIterator<T>::operator==(const JsonValueArrayIterator<T>& other) const
    {
        return arrayIterator == other.arrayIterator && arrayEndIterator == other.arrayEndIterator;
    }

    template<class T>
    bool JsonValueArrayIterator<T>::operator!=(const JsonValueArrayIterator<T>& other) const
    {
        return !(*this == other);
    }

    template<class T>
    T JsonValueArrayIterator<T>::operator*() const
    {
        return arrayIterator->Get<T>();
    }

    template<class T>
    const T* JsonValueArrayIterator<T>::operator->() const
    {
        return &arrayIterator->Get<T>();
    }

    template<class T>
    JsonValueArrayIterator<T>& JsonValueArrayIterator<T>::operator++()
    {
        ++arrayIterator;

        if (arrayIterator != arrayEndIterator && !arrayIterator->Is<T>())
            arrayIterator.SetError();

        return *this;
    }

    template<class T>
    JsonValueArrayIterator<T> JsonValueArrayIterator<T>::operator++(int)
    {
        JsonValueArrayIterator<T> result(*this);
        ++*this;
        return result;
    }
}

#endif
