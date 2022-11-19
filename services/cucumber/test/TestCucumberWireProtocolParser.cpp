#include "services/cucumber/CucumberWireProtocolParser.hpp"
#include "services/cucumber/CucumberStep.hpp"
#include "gmock/gmock.h"

namespace
{
    class CucumberStepStub
        : public services::CucumberStep
    {
    public:
        explicit CucumberStepStub(infra::BoundedConstString step)
            : services::CucumberStep(step, "")
        {}

        void Invoke(infra::JsonArray& arguments) const override
        {}
    };
}

TEST(CucumberStepMatcherTest, should_match_input_for_matching_steps)
{
    std::vector<std::pair<std::string, std::string>> testVectors = {
        std::make_pair("literal step", "literal step"),
        std::make_pair("step with %d argument", "step with 42 argument"),
        std::make_pair("step with %b argument", "step with true argument"),
        std::make_pair("step with '%s' argument", "step with 'foo' argument"),
        std::make_pair("ends on parameter '%s'", "ends on parameter 'foo'"),
        std::make_pair("ends on parameter %d", "ends on parameter 42"),
        std::make_pair("ends on parameter %b", "ends on parameter false"),
        std::make_pair("'%s' starts with string", "'foo' starts with string"),
        std::make_pair("%d starts with integer", "42 starts with integer"),
        std::make_pair("%b starts with integer", "true starts with integer"),
        std::make_pair("'%s'", "'foo'"),
        std::make_pair("%d", "42"),
        std::make_pair("%b", "false"),
        std::make_pair("'%d'", "'42'"),
        std::make_pair("'%b'", "'true'"),
        std::make_pair("'%s' %d %b '%s'", "'foo' 42 false 'bar'"),
        std::make_pair("'%s'", "'foo bar'"),
        std::make_pair("", "")
    };

    for (const auto& [step, match] : testVectors)
    {
        CucumberStepStub s{ step };
        EXPECT_TRUE(s.Matches(match));
    }
}

TEST(CucumberStepMatcherTest, should_not_match_input_for_non_matching_steps)
{
    std::vector<std::pair<std::string, std::string>> testVectors = {
        std::make_pair("step", ""),
        std::make_pair("'%s'", "42"),
        std::make_pair("step with %s argument", "step with foo argument"),
        std::make_pair("'%s' step", "'foo step"),
        std::make_pair("%b step", "foles step"),
        std::make_pair("%b step", "maybe step")
    };

    for (const auto& [step, match] : testVectors)
    {
        CucumberStepStub s{ step };
        EXPECT_FALSE(s.Matches(match));
    }
}

TEST(CucumberStepMatcherTest, should_report_if_step_has_arguments)
{
    CucumberStepStub withArguments{ "I am here for an argument '%s'" };
    EXPECT_TRUE(withArguments.HasArguments());

    CucumberStepStub withoutArguments{ "Me too!" };
    EXPECT_FALSE(withoutArguments.HasArguments());
}

TEST(CucumberStepMatcherTest, should_report_correct_argument_count)
{
    CucumberStepStub withArguments{ "I am here for an argument '%s', %d, %b" };
    EXPECT_EQ(3, withArguments.NrArguments());

    CucumberStepStub withoutArguments{ "Me too!" };
    EXPECT_EQ(0, withoutArguments.NrArguments());
}

TEST(CucumberStepMatcherTest, get_table_argument)
{
    CucumberStepStub withTableArgument{ "I am here for a table argument" };
    infra::JsonArray arguments(R"([[["field","value"]]])");
    withTableArgument.Invoke(arguments);
    infra::BoundedString::WithStorage<5> argument;
    withTableArgument.GetTableArgument(arguments, "field")->ToString(argument);
    EXPECT_EQ("value", argument);
}

TEST(CucumberStepMatcherTest, get_string_argument)
{
    CucumberStepStub withStingArgument{ "I am here for a string argument '%s'" };
    infra::JsonArray arguments(R"([ "abc" ])");
    withStingArgument.Invoke(arguments);
    infra::BoundedString::WithStorage<3> argument;
    withStingArgument.GetStringArgument(arguments, 0)->ToString(argument);
    EXPECT_EQ("abc", argument);
}

TEST(CucumberStepMatcherTest, get_unsigned_integer_argument)
{
    CucumberStepStub withUIntegerArgument{ "I am here for an unsigned integer argument %d" };
    infra::JsonArray arguments(R"([ "5" ])");
    withUIntegerArgument.Invoke(arguments);
    auto argument = withUIntegerArgument.GetUIntegerArgument(arguments, 0);
    EXPECT_EQ(5, *argument);
}

TEST(CucumberStepMatcherTest, get_boolean_argument)
{
    CucumberStepStub withBooleanArgument{ "I am here for an boolean argument %b and %b" };
    infra::JsonArray arguments(R"([ "true", "false" ])");
    withBooleanArgument.Invoke(arguments);
    auto argument = withBooleanArgument.GetBooleanArgument(arguments, 0);
    EXPECT_EQ(true, *argument);
    argument = withBooleanArgument.GetBooleanArgument(arguments, 1);
    EXPECT_EQ(false, *argument);
}

TEST(CucumberStepMatcherTest, get_all_arguments)
{
    CucumberStepStub withDifferingArguments{ "I am here for an unsigned integer argument %d, string argument '%s', boolean argument %b, and table argument" };
    infra::JsonArray arguments(R"([ "5", "abc", "true", [["field","value"]] ])");
    withDifferingArguments.Invoke(arguments);

    auto uIntegerArgument = withDifferingArguments.GetUIntegerArgument(arguments, 0);
    EXPECT_EQ(5, *uIntegerArgument);

    infra::BoundedString::WithStorage<3> stringArgument;
    withDifferingArguments.GetStringArgument(arguments, 1)->ToString(stringArgument);
    EXPECT_EQ("abc", stringArgument);

    auto booleanArgument = withDifferingArguments.GetBooleanArgument(arguments, 2);
    EXPECT_EQ(true, *booleanArgument);

    infra::BoundedString::WithStorage<5> tableArgument;
    withDifferingArguments.GetTableArgument(arguments, "field")->ToString(tableArgument);
    EXPECT_EQ("value", tableArgument);
}

class CucumberWireProtocolParserTest
    : public testing::Test
{
public:
    services::CucumberWireProtocolParser parser;
};

TEST_F(CucumberWireProtocolParserTest, test_valid_input)
{
    EXPECT_TRUE(parser.Valid(R"(["request", {"argument":"argument"}])"));
    EXPECT_FALSE(parser.Valid("invalid input"));
}

TEST_F(CucumberWireProtocolParserTest, should_parse_begin_scenario_tags)
{
    parser.ParseRequest(R"(["begin_scenario",{"tags":["tag1","tag2"]}])");
    EXPECT_EQ(parser.scenarioTags, infra::JsonObject(R"({"tags":["tag1","tag2"]})"));
}
