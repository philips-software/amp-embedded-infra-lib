#ifndef INFRA_JSON_STREAMING_PARSER_HPP
#define INFRA_JSON_STREAMING_PARSER_HPP

#include "infra/util/BoundedString.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/WithStorage.hpp"

namespace infra
{
    class JsonSubObjectParser;
    class JsonSubArrayParser;

    class JsonVisitor
    {
    protected:
        JsonVisitor() = default;
        JsonVisitor(const JsonVisitor& other) = delete;
        JsonVisitor& operator=(const JsonVisitor& other) = delete;
        ~JsonVisitor() = default;

    public:
        virtual void Close() = 0;
        virtual void ParseError() = 0;
        virtual void SemanticError() = 0;
        virtual void StringOverflow() = 0;
    };

    class JsonArrayVisitor;

    class JsonObjectVisitor
        : public JsonVisitor
    {
    public:
        virtual void VisitString(infra::BoundedConstString tag, infra::BoundedConstString value);
        virtual void VisitNumber(infra::BoundedConstString tag, int64_t value);
        virtual void VisitBoolean(infra::BoundedConstString tag, bool value);
        virtual void VisitNull(infra::BoundedConstString tag);
        virtual JsonObjectVisitor* VisitObject(infra::BoundedConstString tag, JsonSubObjectParser& parser);
        virtual JsonArrayVisitor* VisitArray(infra::BoundedConstString tag, JsonSubArrayParser& parser);

        virtual void Close() override;
        virtual void ParseError() override;
        virtual void SemanticError() override;
        virtual void StringOverflow() override;
    };

    class JsonArrayVisitor
        : public JsonVisitor
    {
    protected:
        JsonArrayVisitor() = default;
        JsonArrayVisitor(const JsonArrayVisitor& other) = delete;
        JsonArrayVisitor& operator=(const JsonArrayVisitor& other) = delete;
        ~JsonArrayVisitor() = default;

    public:
        virtual void VisitString(infra::BoundedConstString value);
        virtual void VisitNumber(int64_t value);
        virtual void VisitBoolean(bool value);
        virtual void VisitNull();
        virtual JsonObjectVisitor* VisitObject(JsonSubObjectParser& parser);
        virtual JsonArrayVisitor* VisitArray(JsonSubArrayParser& parser);

        virtual void Close() override;
        virtual void ParseError() override;
        virtual void SemanticError() override;
        virtual void StringOverflow() override;
    };

    class JsonSubParser
    {
    public:
        // The GCC compiler delivered with WICED 4.2 crashes when using a PolymorphicVariant as parameter to a constructor
        // In order to avoid that crash, we use a char& instead
        JsonSubParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer, char& subObjects);
        JsonSubParser(const JsonSubParser& other) = delete;
        JsonSubParser& operator=(const JsonSubParser& other) = delete;
        virtual ~JsonSubParser();

    public:
        virtual void Feed(infra::MemoryRange<const char>& data) = 0;
        virtual JsonVisitor& Visitor() = 0;
        virtual void SetVisitor(JsonVisitor& visitor) = 0;

    protected:
        enum class TokenState
        {
            open,
            done,
            stringOpen,
            stringOpenAndEscaped,
            stringOverflowOpen,
            unicodeValueOpen,
            numberOpen,
            numberFractionalOpen,
            numberExponentOpen,
            identifierOpen
        };

        enum class Token
        {
            error,
            colon,
            comma,
            leftBrace,
            rightBrace,
            leftBracket,
            rightBracket,
            string,
            stringOverflow,
            number,
            true_,
            false_,
            null
        };

    protected:
        void FeedToken(infra::MemoryRange<const char>& data, bool saveValue);
        void ReportParseError();
        void ReportSemanticError();
        infra::BoundedString CopyAndClear(infra::BoundedString& value) const;

    private:
        void FoundToken(Token found);
        void ProcessEscapedData(char c, bool saveValue);
        void AddToValueBuffer(char c, bool saveValue, bool inString);

    protected:
        infra::BoundedString tagBuffer;
        infra::BoundedString valueBuffer;
        infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>& subObjects;

        TokenState tokenState = TokenState::open;
        Token token = Token::error;
        int64_t tokenNumber;
        int8_t tokenSign;

    private:
        uint8_t unicodeIndex = 0;
        uint16_t unicode = 0;
        bool* destructedIndication = nullptr;
    };

    class JsonSubObjectParser
        : public JsonSubParser
    {
    public:
        JsonSubObjectParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer, char& subObjects, JsonObjectVisitor& visitor);
        JsonSubObjectParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer, char& subObjects);
        ~JsonSubObjectParser();

        virtual void Feed(infra::MemoryRange<const char>& data) override;
        virtual JsonVisitor& Visitor() override;
        virtual void SetVisitor(JsonVisitor& visitor) override;

        void SemanticError();

    private:
        enum class State
        {
            init,
            initialOpen,
            open,
            tagClosed,
            valueExpected,
            closed,
            skipNestedObject,
            skipNestedArray,
            parseError,
            semanticError
        };

    private:
        JsonObjectVisitor* visitor;
        State state;
        uint32_t skipSubObjects = 0;
        bool* destructedIndication = nullptr;
    };

    class JsonSubArrayParser
        : public JsonSubParser
    {
    public:
        JsonSubArrayParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer, char& subObjects, JsonArrayVisitor& visitor);
        JsonSubArrayParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer, char& subObjects);
        ~JsonSubArrayParser();

        virtual void Feed(infra::MemoryRange<const char>& data) override;
        virtual JsonVisitor& Visitor() override;
        virtual void SetVisitor(JsonVisitor& visitor) override;

        void SemanticError();

        enum class State
        {
            init,
            initialOpen,
            open,
            closed,
            skipNestedObject,
            skipNestedArray,
            parseError,
            semanticError
        };

    private:
        JsonArrayVisitor* visitor;

        State state = State::initialOpen;
        uint32_t skipSubObjects = 0;
        bool* destructedIndication = nullptr;
    };

    class JsonStreamingObjectParser
    {
    public:
        template<std::size_t MaxTagLength, std::size_t MaxValueLength, std::size_t MaxObjectNesting>
            struct WithBuffers;

        JsonStreamingObjectParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer, char& subObjects, JsonObjectVisitor& visitor);

        void Feed(infra::BoundedConstString data);

    private:
        infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>& subObjects;
    };

    class JsonStreamingArrayParser
    {
    public:
        template<std::size_t MaxTagLength, std::size_t MaxValueLength, std::size_t MaxObjectNesting>
            struct WithBuffers;

            JsonStreamingArrayParser(infra::BoundedString tagBuffer, infra::BoundedString valueBuffer, char& subObjects, JsonArrayVisitor& visitor);

        void Feed(infra::BoundedConstString data);

    private:
        infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>& subObjects;
    };

    template<std::size_t MaxTagLength, std::size_t MaxValueLength, std::size_t MaxObjectNesting>
    struct Buffers
    {
        infra::BoundedString::WithStorage<MaxTagLength> tagBufferStorage;
        infra::BoundedString::WithStorage<MaxValueLength> valueBufferStorage;
        infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>::WithMaxSize<MaxObjectNesting> subObjectsStorage;
    };

    template<std::size_t MaxTagLength, std::size_t MaxValueLength, std::size_t MaxObjectNesting>
    struct JsonStreamingObjectParser::WithBuffers
        : public Buffers<MaxTagLength, MaxValueLength, MaxObjectNesting>
        , public JsonStreamingObjectParser
    {
        WithBuffers(JsonObjectVisitor& visitor);
    };

    template<std::size_t MaxTagLength, std::size_t MaxValueLength, std::size_t MaxObjectNesting>
    struct JsonStreamingArrayParser::WithBuffers
        : public Buffers<MaxTagLength, MaxValueLength, MaxObjectNesting>
        , public JsonStreamingArrayParser
    {
        WithBuffers(JsonArrayVisitor& visitor);
    };

    class JsonObjectVisitorDecorator
        : public JsonObjectVisitor
    {
    protected:
        JsonObjectVisitorDecorator(JsonObjectVisitor& decorated);

    public:
        virtual void VisitString(infra::BoundedConstString tag, infra::BoundedConstString value) override;
        virtual void VisitNumber(infra::BoundedConstString tag, int64_t value) override;
        virtual void VisitBoolean(infra::BoundedConstString tag, bool value) override;
        virtual void VisitNull(infra::BoundedConstString tag) override;
        virtual JsonObjectVisitor* VisitObject(infra::BoundedConstString tag, JsonSubObjectParser& parser) override;
        virtual JsonArrayVisitor* VisitArray(infra::BoundedConstString tag, JsonSubArrayParser& parser) override;

        virtual void Close() override;
        virtual void ParseError() override;
        virtual void SemanticError() override;
        virtual void StringOverflow() override;

    private:
        JsonObjectVisitor& decorated;
    };

    //// Implementation

    template<std::size_t MaxTagLength, std::size_t MaxValueLength, std::size_t MaxObjectNesting>
    JsonStreamingObjectParser::WithBuffers<MaxTagLength, MaxValueLength, MaxObjectNesting>::WithBuffers(JsonObjectVisitor& visitor)
        : JsonStreamingObjectParser(this->tagBufferStorage, this->valueBufferStorage,
            reinterpret_cast<char&>(static_cast<infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>&>(this->subObjectsStorage)),
            visitor)
    {}

    template<std::size_t MaxTagLength, std::size_t MaxValueLength, std::size_t MaxObjectNesting>
    JsonStreamingArrayParser::WithBuffers<MaxTagLength, MaxValueLength, MaxObjectNesting>::WithBuffers(JsonArrayVisitor& visitor)
        : JsonStreamingArrayParser(this->tagBufferStorage, this->valueBufferStorage,
            reinterpret_cast<char&>(static_cast<infra::BoundedVector<infra::PolymorphicVariant<JsonSubParser, JsonSubObjectParser, JsonSubArrayParser>>&>(this->subObjectsStorage)),
            visitor)
    {}
}

#endif
