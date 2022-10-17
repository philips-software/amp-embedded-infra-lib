#ifndef INFRA_JSON_FORMATTER_HPP
#define INFRA_JSON_FORMATTER_HPP

#include "infra/stream/StringOutputStream.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/WithStorage.hpp"
#include <type_traits>

namespace infra
{
    std::size_t JsonEscapedStringSize(infra::BoundedConstString string);
    infra::BoundedConstString JsonSubStringOfMaxEscapedSize(infra::BoundedConstString string, std::size_t maxEscapedSize);

    class JsonArrayFormatter;
    class JsonStringStream;
    
    class JsonObjectFormatter
    {
    public:
        using WithStringStream = infra::WithStorage<JsonObjectFormatter, infra::StringOutputStream>;

        explicit JsonObjectFormatter(infra::TextOutputStream& stream);
        JsonObjectFormatter(const JsonObjectFormatter& other) = delete;
        JsonObjectFormatter(JsonObjectFormatter&& other) noexcept;
        JsonObjectFormatter& operator=(const JsonObjectFormatter& other) = delete;
        JsonObjectFormatter& operator=(JsonObjectFormatter&& other) noexcept;
        ~JsonObjectFormatter();

        void Add(const char* tagName, bool tag);
        void Add(JsonString tagName, bool tag);
        void Add(const char* tagName, int32_t tag);
        void Add(JsonString tagName, int32_t tag);
        void Add(const char* tagName, uint32_t tag);
        void Add(const char* tagName, int64_t tag);
        void Add(JsonString tagName, int64_t tag);
        void Add(const char* tagName, uint64_t tag);
        void Add(JsonString tagName, uint64_t tag);
        void Add(const char* tagName, JsonBiggerInt tag);
        void Add(JsonString tagName, JsonBiggerInt tag);
        void Add(const char* tagName, const char* tag);
        void Add(const char* tagName, infra::BoundedConstString tag);
        void Add(infra::BoundedConstString tagName, infra::BoundedConstString tag);
        void Add(JsonString tagName, JsonString tag);
        void Add(JsonString tagName, const JsonObject& tag);
        void Add(JsonString tagName, const JsonArray& tag);
        void Add(JsonString tagName, const JsonValue& tag);
        void Add(const infra::JsonKeyValue& keyValue);

        template<class T>
        struct IntegralOrEnum
        {
            static constexpr bool value = std::is_integral<T>::value || std::is_enum<T>::value;
        };

        template<class T, typename std::enable_if<IntegralOrEnum<T>::value, T>::type* = nullptr>
        void Add(const char* tagName, T v)
        {
            using type = typename infra::NormalizedIntegralType<T>::type;

            return Add(tagName, static_cast<type>(v));
        }

        template<class T, typename std::enable_if<IntegralOrEnum<T>::value, T>::type* = nullptr>
        void Add(JsonString tagName, T v)
        {
            using type = typename infra::NormalizedIntegralType<T>::type;

            return Add(tagName, static_cast<type>(v));
        }

        void AddMilliFloat(const char* tagName, uint32_t intValue, uint32_t milliFractionalValue);
        void AddMilliFloat(infra::JsonString tagName, uint32_t intValue, uint32_t milliFractionalValue);
        void AddSubObject(const char* tagName, infra::BoundedConstString json);
        JsonObjectFormatter SubObject(const char* tagName);
        JsonObjectFormatter SubObject(JsonString tagName);
        JsonArrayFormatter SubArray(infra::BoundedConstString tagName);
        JsonStringStream AddString(const char* tagName);
        infra::TextOutputStream AddObject(const char* tagName);

        bool Failed() const;

    private:
        void InsertSeparation();

    private:
        infra::TextOutputStream* stream;
        bool empty = true;
    };

    void Merge(infra::JsonObjectFormatter& formatter, infra::JsonObject& object, infra::BoundedConstString path,const infra::JsonValue& valueToMerge);

    class JsonArrayFormatter
    {
    public:
        using WithStringStream = infra::WithStorage<JsonArrayFormatter, infra::StringOutputStream>;

        explicit JsonArrayFormatter(infra::TextOutputStream& stream);
        JsonArrayFormatter(const JsonArrayFormatter& other) = delete;
        JsonArrayFormatter(JsonArrayFormatter&& other) noexcept;
        JsonArrayFormatter& operator=(const JsonArrayFormatter& other) = delete;
        JsonArrayFormatter& operator=(JsonArrayFormatter&& other) noexcept;
        ~JsonArrayFormatter();

        void Add(bool tag);
        void Add(int32_t tag);
        void Add(uint32_t tag);
        void Add(int64_t tag);
        void Add(JsonBiggerInt tag);
        void Add(const char* tag);
        void Add(infra::BoundedConstString tag);
        JsonObjectFormatter SubObject();
        JsonArrayFormatter SubArray();

        bool Failed() const;

    private:
        void InsertSeparation();

    private:
        infra::TextOutputStream* stream;
        bool empty = true;
    };

    class JsonStringStream
        : public infra::TextOutputStream
    {
    public:
        explicit JsonStringStream(infra::TextOutputStream& stream);
        JsonStringStream(const JsonStringStream& other) = delete;
        JsonStringStream(JsonStringStream&& other) noexcept;
        JsonStringStream& operator=(const JsonStringStream& other) = delete;
        JsonStringStream& operator=(JsonStringStream&& other) noexcept;
        ~JsonStringStream();

    private:
        bool owned = true;
    };
}

#endif
