#include "infra/syntax/JsonStreamingParser.hpp"
#include "infra/syntax/test_doubles/JsonStreamingParserMock.hpp"
#include "infra/util/test_helper/BoundedStringMatcher.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "gmock/gmock.h"

namespace
{
    template<class T>
    void DeleteValue(T& value)
    {
        value.~T();
    }
}

class JsonStreamingObjectParserTest
    : public testing::Test
{
public:
    testing::StrictMock<infra::JsonObjectVisitorMock> visitor;
    testing::StrictMock<infra::JsonObjectVisitorMock> nestedVisitor;
    infra::JsonStreamingObjectParser::WithBuffers<8, 12, 2> parser{ visitor };
};

class JsonStreamingObjectParserWith3LevelsTest
    : public testing::Test
{
public:
    testing::StrictMock<infra::JsonObjectVisitorMock> visitor;
    testing::StrictMock<infra::JsonObjectVisitorMock> nestedVisitor;
    infra::JsonStreamingObjectParser::WithBuffers<8, 12, 3> parser{ visitor };
};

TEST_F(JsonStreamingObjectParserTest, feed_empty_object)
{
    EXPECT_CALL(visitor, Close());
    parser.Feed(" {}");
}

TEST_F(JsonStreamingObjectParserTest, delete_on_Close)
{
    EXPECT_CALL(visitor, Close()).WillOnce(testing::Invoke([this]()
        {
            DeleteValue(parser);
        }));
    parser.Feed("{}");

    new (&parser) decltype(parser){ visitor };
}

TEST_F(JsonStreamingObjectParserTest, non_open_object_results_in_ParseError)
{
    EXPECT_CALL(visitor, ParseError());
    parser.Feed("}");
}

TEST_F(JsonStreamingObjectParserTest, feed_parts)
{
    parser.Feed("{");
    parser.Feed("     ");
    EXPECT_CALL(visitor, Close());
    parser.Feed("}");
}

TEST_F(JsonStreamingObjectParserTest, feed_garbage)
{
    EXPECT_CALL(visitor, ParseError());
    parser.Feed("{ ^");
}

TEST_F(JsonStreamingObjectParserTest, VisitString)
{
    EXPECT_CALL(visitor, VisitString("a", "b"));
    parser.Feed(R"({ "a" : "b")");
}

TEST_F(JsonStreamingObjectParserTest, VisitBoolean)
{
    EXPECT_CALL(visitor, VisitBoolean("a", false));
    parser.Feed(R"({ "a" : false )");

    EXPECT_CALL(visitor, VisitBoolean("a", true));
    parser.Feed(R"(, "a" : true )");
}

TEST_F(JsonStreamingObjectParserTest, VisitNull)
{
    EXPECT_CALL(visitor, VisitNull("a"));
    parser.Feed(R"({ "a" : null )");
}

TEST_F(JsonStreamingObjectParserTest, VisitNumber)
{
    EXPECT_CALL(visitor, VisitNumber("a", 5));
    parser.Feed(R"({ "a" : 5,)");

    EXPECT_CALL(visitor, VisitNumber("a", 16));
    parser.Feed(R"( "a" : 16,)");

    EXPECT_CALL(visitor, VisitNumber("a", -823));
    parser.Feed(R"( "a" : -823,)");

    EXPECT_CALL(visitor, VisitNumber("a", 5));
    parser.Feed(R"( "a" : 5.123,)");

    EXPECT_CALL(visitor, VisitNumber("a", 5));
    parser.Feed(R"( "a" : 5.123e456,)");

    EXPECT_CALL(visitor, VisitNumber("a", 5));
    parser.Feed(R"( "a" : 5.123E456,)");

    EXPECT_CALL(visitor, VisitNumber("a", 5));
    parser.Feed(R"( "a" : 5.123e+456,)");

    EXPECT_CALL(visitor, VisitNumber("a", 5));
    parser.Feed(R"( "a" : 5.123e-456,)");
}

TEST_F(JsonStreamingObjectParserTest, unknown_identifier_results_in_ParseError)
{
    EXPECT_CALL(visitor, ParseError());
    parser.Feed(R"({ "a" : bla )");
}

TEST_F(JsonStreamingObjectParserTest, too_long_identifier_results_in_ParseError)
{
    EXPECT_CALL(visitor, ParseError());
    parser.Feed(R"({ "a" : abcdefghijklmnopqrstuvwyz)");
}

TEST_F(JsonStreamingObjectParserTest, VisitString_with_escape_in_tag)
{
    EXPECT_CALL(visitor, VisitString("a\"\\\b\f\n\r\t", "b"));
    parser.Feed(R"({ "a\"\\\b\f\n\r\t" : "b")");
}

TEST_F(JsonStreamingObjectParserTest, VisitString_with_escape_in_value)
{
    EXPECT_CALL(visitor, VisitString("a", "b\"\\\b\f\n\r\t"));
    parser.Feed(R"({ "a" : "b\"\\\b\f\n\r\t")");

    EXPECT_CALL(visitor, VisitString("a", " "));
    parser.Feed(R"(, "a" : "\u0020")");

    EXPECT_CALL(visitor, VisitString("a", "*"));
    parser.Feed(R"(, "a" : "\u002a")");

    EXPECT_CALL(visitor, VisitString("a", "*"));
    parser.Feed(R"(, "a" : "\u002A")");

    EXPECT_CALL(visitor, ParseError());
    parser.Feed(R"(, "a" : "\u002x")");
}

TEST_F(JsonStreamingObjectParserTest, VisitString_twice)
{
    EXPECT_CALL(visitor, VisitString("a", "b"));
    parser.Feed(R"({ "a" : "b")");

    EXPECT_CALL(visitor, VisitString("a", "b"));
    parser.Feed(R"(, "a" : "b")");
}

TEST_F(JsonStreamingObjectParserTest, destruct_parser_during_callback)
{
    EXPECT_CALL(visitor, VisitString("a", "b")).WillOnce(testing::Invoke([this](infra::BoundedConstString, infra::BoundedConstString)
        {
            DeleteValue(parser);
        }));
    parser.Feed(R"({ "a" : "b", "a" : "b")");

    new (&parser) decltype(parser){ visitor };
}

TEST_F(JsonStreamingObjectParserTest, VisitObject)
{
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Return(&nestedVisitor));
    parser.Feed(R"({ "a" : {)");

    EXPECT_CALL(nestedVisitor, VisitString("a", "b"));
    parser.Feed(R"("a" : "b")");

    EXPECT_CALL(nestedVisitor, Close());
    EXPECT_CALL(visitor, VisitString("a", "b"));
    parser.Feed(R"(}, "a" : "b")");
}

TEST_F(JsonStreamingObjectParserTest, delete_on_VisitObject)
{
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString tag, infra::JsonSubObjectParser& parser)
        {
            DeleteValue(this->parser);
            return nullptr;
        }));
    parser.Feed(R"({ "a" : {)");

    new (&parser) decltype(parser){ visitor };
}

TEST_F(JsonStreamingObjectParserTest, nested_objects_above_limit_are_skipped)
{
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Return(&nestedVisitor));
    parser.Feed(R"({ "a" : {)");

    parser.Feed(R"("a" : {})");
}

TEST_F(JsonStreamingObjectParserTest, nested_arrays_above_limit_are_skipped)
{
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Return(&nestedVisitor));
    parser.Feed(R"({ "a" : {)");

    parser.Feed(R"("a" : [])");
}

TEST_F(JsonStreamingObjectParserTest, ParseError_propagates_in_object_hierarchy)
{
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Return(&nestedVisitor));
    parser.Feed(R"({ "a" : {)");

    EXPECT_CALL(nestedVisitor, ParseError());
    EXPECT_CALL(visitor, ParseError());
    parser.Feed("{ ^");
}

TEST_F(JsonStreamingObjectParserTest, nested_visitor_invokes_SemanticError)
{
    infra::JsonSubObjectParser* nestedParser = nullptr;
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Invoke([&nestedParser, this](infra::BoundedConstString tag, infra::JsonSubObjectParser& subObjectParser)
        {
            nestedParser = &subObjectParser;
            return &nestedVisitor;
        }));
    parser.Feed(R"({ "a" : {)");

    EXPECT_CALL(visitor, SemanticError());
    EXPECT_CALL(nestedVisitor, VisitString("a", "b")).WillOnce(testing::Invoke([nestedParser](infra::BoundedConstString tag, infra::BoundedConstString value)
        {
            nestedParser->SemanticError();
        }));
    parser.Feed(R"("a" : "b")");
}

TEST_F(JsonStreamingObjectParserTest, propagation_stops_when_parser_is_deleted_in_ParserError)
{
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Return(&nestedVisitor));
    parser.Feed(R"({ "a" : {)");

    EXPECT_CALL(nestedVisitor, ParseError()).WillOnce(testing::Invoke([this]()
        {
            infra::ReConstruct(parser, visitor);
        }));
    parser.Feed("{ ^");
}

TEST_F(JsonStreamingObjectParserWith3LevelsTest, propagation_stops_when_parser_is_deleted_in_SemanticError)
{
    infra::JsonSubObjectParser* nestedParser = nullptr;
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Invoke([&nestedParser, this](infra::BoundedConstString tag, infra::JsonSubObjectParser& subObjectParser)
        {
            nestedParser = &subObjectParser;
            return &nestedVisitor;
        }));
    parser.Feed(R"({ "a" : {)");

    testing::StrictMock<infra::JsonObjectVisitorMock> nestedVisitor2;
    EXPECT_CALL(nestedVisitor, VisitObject("a", testing::_)).WillOnce(testing::Invoke([&nestedParser, this, &nestedVisitor2](infra::BoundedConstString tag, infra::JsonSubObjectParser& subObjectParser)
        {
            nestedParser = &subObjectParser;
            return &nestedVisitor2;
        }));
    parser.Feed(R"("a" : {)");

    EXPECT_CALL(nestedVisitor, SemanticError()).WillOnce(testing::Invoke([this]()
        {
            infra::ReConstruct(parser, visitor);
        }));
    EXPECT_CALL(nestedVisitor2, VisitString("a", "b")).WillOnce(testing::Invoke([nestedParser](infra::BoundedConstString tag, infra::BoundedConstString value)
        {
            nestedParser->SemanticError();
        }));
    parser.Feed(R"("a" : "b")");
}

TEST_F(JsonStreamingObjectParserTest, overflow_in_value_results_in_StringOverflow)
{
    EXPECT_CALL(visitor, StringOverflow());
    parser.Feed(R"({ "a" : "1234567890123" )");
}

TEST_F(JsonStreamingObjectParserTest, overflow_in_tag_is_truncated)
{
    EXPECT_CALL(visitor, VisitString("12345678", "a"));
    parser.Feed(R"({ "1234567890123" : "a" )");
}

TEST_F(JsonStreamingObjectParserTest, when_nested_visitor_is_null_subobject_is_skipped)
{
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Return(nullptr));
    parser.Feed(R"({ "a" : {)");

    parser.Feed(R"("a" : "b")");

    EXPECT_CALL(visitor, VisitString("a", "b"));
    parser.Feed(R"(}, "a" : "b")");
}

TEST_F(JsonStreamingObjectParserTest, when_nested_visitor_is_null_all_subobjects_are_skipped)
{
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Return(nullptr));
    parser.Feed(R"({ "a" : {)");

    parser.Feed(R"("sub": {"a" : "b"})");

    EXPECT_CALL(visitor, VisitString("a", "b"));
    parser.Feed(R"(}, "a" : "b")");
}

TEST_F(JsonStreamingObjectParserTest, when_nested_visitor_is_null_subarray_is_skipped)
{
    EXPECT_CALL(visitor, VisitArray("a", testing::_)).WillOnce(testing::Return(nullptr));
    parser.Feed(R"({ "a" : [)");

    parser.Feed(R"("b")");

    EXPECT_CALL(visitor, VisitString("a", "b"));
    parser.Feed(R"(], "a" : "b")");
}

TEST_F(JsonStreamingObjectParserTest, delete_on_VisitArray)
{
    EXPECT_CALL(visitor, VisitArray("a", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString tag, infra::JsonSubArrayParser& parser)
        {
            DeleteValue(this->parser);
            return nullptr;
        }));
    parser.Feed(R"({ "a" : [)");

    new (&parser) decltype(parser){ visitor };
}

TEST_F(JsonStreamingObjectParserTest, when_nested_visitor_is_null_all_subarrays_are_skipped)
{
    EXPECT_CALL(visitor, VisitArray("a", testing::_)).WillOnce(testing::Return(nullptr));
    parser.Feed(R"({ "a" : [)");

    parser.Feed(R"([])");

    EXPECT_CALL(visitor, VisitString("a", "b"));
    parser.Feed(R"(], "a" : "b")");
}

TEST_F(JsonStreamingObjectParserTest, when_nested_visitor_is_null_tokens_of_subobjects_dont_overflow)
{
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Return(nullptr));
    parser.Feed(R"({ "a" : {)");

    parser.Feed(R"("abcdefghijklmnopqrstuvwxyz\u0000" : "abcdefghijklmnopqrstuvwxyz\u0000")");

    EXPECT_CALL(visitor, VisitString("a", "b"));
    parser.Feed(R"(}, "a" : "b")");
}

TEST_F(JsonStreamingObjectParserTest, VisitAllTypes)
{
    testing::StrictMock<infra::JsonObjectVisitorMock> nestedObjectVisitor;
    testing::StrictMock<infra::JsonArrayVisitorMock> nestedArrayVisitor;

    EXPECT_CALL(visitor, VisitString("a", "a"));
    EXPECT_CALL(visitor, VisitString("a", "b"));
    EXPECT_CALL(visitor, VisitBoolean("a", false));
    EXPECT_CALL(visitor, VisitBoolean("a", true));
    EXPECT_CALL(visitor, VisitNull("a"));
    EXPECT_CALL(visitor, VisitNumber("a", 5));
    EXPECT_CALL(visitor, VisitObject("a", testing::_)).WillOnce(testing::Return(&nestedObjectVisitor));
    EXPECT_CALL(nestedObjectVisitor, Close());
    EXPECT_CALL(visitor, VisitArray("a", testing::_)).WillOnce(testing::Return(&nestedArrayVisitor));
    EXPECT_CALL(nestedArrayVisitor, Close());
    EXPECT_CALL(visitor, Close());
    parser.Feed(R"({ "a":"a", "a":"b", "a":false, "a":true, "a":null, "a":5, "a":{}, "a":[] }")");
}

class JsonStreamingObjectParserArrayTest
    : public testing::Test
{
public:
    JsonStreamingObjectParserArrayTest()
    {
        EXPECT_CALL(visitor, VisitArray("a", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString tag, infra::JsonSubArrayParser& subArrayParser)
            {
                nestedParser = &subArrayParser;
                return &arrayVisitor;
            }));
        parser.Feed(R"({ "a" : [)");
    }

    testing::StrictMock<infra::JsonObjectVisitorMock> visitor;
    testing::StrictMock<infra::JsonObjectVisitorMock> nestedObjectVisitor;
    testing::StrictMock<infra::JsonArrayVisitorMock> nestedArrayVisitor;
    testing::StrictMock<infra::JsonArrayVisitorMock> arrayVisitor;
    infra::JsonSubArrayParser* nestedParser = nullptr;
    infra::JsonStreamingObjectParser::WithBuffers<8, 12, 3> parser{ visitor };
};

TEST_F(JsonStreamingObjectParserArrayTest, feed_garbage)
{
    EXPECT_CALL(arrayVisitor, ParseError());
    EXPECT_CALL(visitor, ParseError());
    parser.Feed("^");
}

TEST_F(JsonStreamingObjectParserArrayTest, VisitString)
{
    EXPECT_CALL(arrayVisitor, VisitString("a"));
    parser.Feed(R"("a")");

    EXPECT_CALL(arrayVisitor, VisitString("b"));
    parser.Feed(R"(, "b")");
}

TEST_F(JsonStreamingObjectParserArrayTest, nested_array_visitor_invokes_SemanticError)
{
    EXPECT_CALL(visitor, SemanticError());
    EXPECT_CALL(arrayVisitor, VisitString("a")).WillOnce(testing::Invoke([this](infra::BoundedConstString value)
        {
            nestedParser->SemanticError();
        }));
    parser.Feed(R"("a" : "b")");
}

TEST_F(JsonStreamingObjectParserArrayTest, VisitBoolean)
{
    EXPECT_CALL(arrayVisitor, VisitBoolean(false));
    parser.Feed(R"(false )");

    EXPECT_CALL(arrayVisitor, VisitBoolean(true));
    parser.Feed(R"(, true )");
}

TEST_F(JsonStreamingObjectParserArrayTest, VisitNull)
{
    EXPECT_CALL(arrayVisitor, VisitNull());
    parser.Feed(R"(null )");
}

TEST_F(JsonStreamingObjectParserArrayTest, VisitNumber)
{
    EXPECT_CALL(arrayVisitor, VisitNumber(5));
    parser.Feed(R"(5,)");
}

TEST_F(JsonStreamingObjectParserArrayTest, close_array)
{
    EXPECT_CALL(arrayVisitor, Close());
    parser.Feed(R"(])");
}

TEST_F(JsonStreamingObjectParserArrayTest, delete_on_Close)
{
    EXPECT_CALL(arrayVisitor, Close()).WillOnce(testing::Invoke([this]()
        {
            DeleteValue(parser);
        }));
    parser.Feed(R"(])");

    new (&parser) decltype(parser){ visitor };
}

TEST_F(JsonStreamingObjectParserArrayTest, array_cannot_be_closed_after_comma)
{
    EXPECT_CALL(arrayVisitor, VisitBoolean(false));
    parser.Feed(R"(false, )");

    EXPECT_CALL(arrayVisitor, ParseError());
    EXPECT_CALL(visitor, ParseError());
    parser.Feed(R"(])");
}

TEST_F(JsonStreamingObjectParserArrayTest, close_array_after_value)
{
    EXPECT_CALL(arrayVisitor, VisitBoolean(false));
    parser.Feed(R"(false )");

    EXPECT_CALL(arrayVisitor, Close());
    parser.Feed(R"(])");
}

TEST_F(JsonStreamingObjectParserArrayTest, VisitObject)
{
    EXPECT_CALL(arrayVisitor, VisitObject(testing::_)).WillOnce(testing::Return(&nestedObjectVisitor));
    parser.Feed(R"({)");

    EXPECT_CALL(nestedObjectVisitor, Close());
    EXPECT_CALL(arrayVisitor, VisitString("a"));
    parser.Feed(R"(}, "a")");
}

TEST_F(JsonStreamingObjectParserArrayTest, delete_on_VisitObject)
{
    EXPECT_CALL(arrayVisitor, VisitObject(testing::_)).WillOnce(testing::Invoke([this](infra::JsonSubObjectParser&)
        {
            DeleteValue(parser);
            return nullptr;
        }));
    parser.Feed(R"({)");

    new (&parser) decltype(parser){ visitor };
}

TEST_F(JsonStreamingObjectParserArrayTest, VisitArray)
{
    EXPECT_CALL(arrayVisitor, VisitArray(testing::_)).WillOnce(testing::Return(&nestedArrayVisitor));
    parser.Feed(R"([)");

    EXPECT_CALL(nestedArrayVisitor, Close());
    EXPECT_CALL(arrayVisitor, VisitString("a"));
    parser.Feed(R"(], "a")");
}

TEST_F(JsonStreamingObjectParserArrayTest, delete_on_VisitArray)
{
    EXPECT_CALL(arrayVisitor, VisitArray(testing::_)).WillOnce(testing::Invoke([this](infra::JsonSubArrayParser&)
        {
            DeleteValue(parser);
            return nullptr;
        }));
    parser.Feed(R"([)");

    new (&parser) decltype(parser){ visitor };
}

TEST_F(JsonStreamingObjectParserArrayTest, VisitAllTypes)
{
    EXPECT_CALL(arrayVisitor, VisitString("a"));
    EXPECT_CALL(arrayVisitor, VisitString("b"));
    EXPECT_CALL(arrayVisitor, VisitBoolean(false));
    EXPECT_CALL(arrayVisitor, VisitBoolean(true));
    EXPECT_CALL(arrayVisitor, VisitNull());
    EXPECT_CALL(arrayVisitor, VisitNumber(5));
    EXPECT_CALL(arrayVisitor, VisitObject(testing::_)).WillOnce(testing::Return(&nestedObjectVisitor));
    EXPECT_CALL(nestedObjectVisitor, Close());
    EXPECT_CALL(arrayVisitor, VisitArray(testing::_)).WillOnce(testing::Return(&nestedArrayVisitor));
    EXPECT_CALL(nestedArrayVisitor, Close());
    parser.Feed(R"("a", "b", false, true, null, 5, {}, []")");
}

TEST_F(JsonStreamingObjectParserArrayTest, nested_objects_above_limit_are_skipped)
{
    EXPECT_CALL(arrayVisitor, VisitArray(testing::_)).WillOnce(testing::Return(&nestedArrayVisitor));
    parser.Feed(R"([)");

    parser.Feed(R"({})");
}

TEST_F(JsonStreamingObjectParserArrayTest, nested_arrays_above_limit_are_skipped)
{
    EXPECT_CALL(arrayVisitor, VisitArray(testing::_)).WillOnce(testing::Return(&nestedArrayVisitor));
    parser.Feed(R"([)");

    parser.Feed(R"([])");
}

TEST_F(JsonStreamingObjectParserArrayTest, when_nested_visitor_is_null_subobject_is_skipped)
{
    EXPECT_CALL(arrayVisitor, VisitObject(testing::_)).WillOnce(testing::Return(nullptr));
    parser.Feed(R"({)");

    parser.Feed(R"("a" : "b")");

    EXPECT_CALL(arrayVisitor, VisitString("a"));
    parser.Feed(R"(}, "a")");
}

TEST_F(JsonStreamingObjectParserArrayTest, when_nested_visitor_is_null_all_subobjects_are_skipped)
{
    EXPECT_CALL(arrayVisitor, VisitObject(testing::_)).WillOnce(testing::Return(nullptr));
    parser.Feed(R"({)");

    parser.Feed(R"("sub": {"a" : "b"})");

    EXPECT_CALL(arrayVisitor, VisitString("a"));
    parser.Feed(R"(}, "a")");
}

TEST_F(JsonStreamingObjectParserArrayTest, when_nested_visitor_is_null_subarray_is_skipped)
{
    EXPECT_CALL(arrayVisitor, VisitArray(testing::_)).WillOnce(testing::Return(nullptr));
    parser.Feed(R"([)");

    parser.Feed(R"("b")");

    EXPECT_CALL(arrayVisitor, VisitString("a"));
    parser.Feed(R"(], "a")");
}

TEST_F(JsonStreamingObjectParserArrayTest, when_nested_visitor_is_null_all_subarrays_are_skipped)
{
    EXPECT_CALL(arrayVisitor, VisitArray(testing::_)).WillOnce(testing::Return(nullptr));
    parser.Feed(R"([)");

    parser.Feed(R"([])");

    EXPECT_CALL(arrayVisitor, VisitString("a"));
    parser.Feed(R"(], "a")");
}

TEST_F(JsonStreamingObjectParserArrayTest, when_nested_visitor_is_null_tokens_of_subobjects_dont_overflow)
{
    EXPECT_CALL(arrayVisitor, VisitObject(testing::_)).WillOnce(testing::Return(nullptr));
    parser.Feed(R"({)");

    parser.Feed(R"("abcdefghijklmnopqrstuvwxyz\u0000" : "abcdefghijklmnopqrstuvwxyz\u0000")");

    EXPECT_CALL(arrayVisitor, VisitString("a"));
    parser.Feed(R"(}, "a")");
}

class JsonStreamingArrayParserTest
    : public testing::Test
{
public:
    testing::StrictMock<infra::JsonArrayVisitorMock> visitor;
    infra::JsonStreamingArrayParser::WithBuffers<8, 12, 2> parser{ visitor };
};

TEST_F(JsonStreamingArrayParserTest, feed_empty_array)
{
    EXPECT_CALL(visitor, Close());
    parser.Feed(" []");
}

TEST_F(JsonStreamingArrayParserTest, VisitString)
{
    EXPECT_CALL(visitor, VisitString("a"));
    parser.Feed(R"([ "a" )");
}
