#ifndef INFRA_JSON_STREAMING_PARSER_MOCK_HPP
#define INFRA_JSON_STREAMING_PARSER_MOCK_HPP

#include "infra/syntax/JsonStreamingParser.hpp"
#include "infra/util/test_helper/BoundedStringMatcher.hpp"
#include "gmock/gmock.h"

namespace infra
{
    class JsonObjectVisitorMock
        : public infra::JsonObjectVisitor
    {
    public:
        MOCK_METHOD2(VisitString, void(BoundedConstString tag, BoundedConstString value));
        MOCK_METHOD2(VisitNumber, void(BoundedConstString tag, int64_t value));
        MOCK_METHOD2(VisitBoolean, void(BoundedConstString tag, bool value));
        MOCK_METHOD1(VisitNull, void(BoundedConstString tag));
        MOCK_METHOD2(VisitObject, JsonObjectVisitor*(BoundedConstString tag, JsonSubObjectParser& parser));
        MOCK_METHOD2(VisitArray, JsonArrayVisitor*(BoundedConstString tag, JsonSubArrayParser& parser));
        MOCK_METHOD0(Close, void());
        MOCK_METHOD0(ParseError, void());
        MOCK_METHOD0(SemanticError, void());
        MOCK_METHOD0(StringOverflow, void());
    };

    class JsonArrayVisitorMock
        : public JsonArrayVisitor
    {
    public:
        MOCK_METHOD1(VisitString, void(BoundedConstString value));
        MOCK_METHOD1(VisitNumber, void(int64_t value));
        MOCK_METHOD1(VisitBoolean, void(bool value));
        MOCK_METHOD0(VisitNull, void());
        MOCK_METHOD1(VisitObject, JsonObjectVisitor*(JsonSubObjectParser& parser));
        MOCK_METHOD1(VisitArray, JsonArrayVisitor*(JsonSubArrayParser& parser));
        MOCK_METHOD0(Close, void());
        MOCK_METHOD0(ParseError, void());
        MOCK_METHOD0(SemanticError, void());
        MOCK_METHOD0(StringOverflow, void());
    };
}

#endif
