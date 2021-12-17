#include "gmock/gmock.h"
#include "services/cucumber/CucumberWireProtocolParser.hpp"

class StepStub
    : public services::CucumberStep
{
public:
    StepStub(infra::BoundedConstString stepName)
        : services::CucumberStep(stepName, "")
    {}

    void Invoke(infra::JsonArray& arguments) final 
    {
        invokeArguments = &arguments;
    };
};

bool CheckStepMatcher(infra::BoundedString stepString, infra::BoundedString matchString)
{
    StepStub step{ stepString };
    return services::CucumberStepStorage::Instance().MatchesStepName(step, matchString);
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

    for (const auto& stepAndMatch : testVectors)
        EXPECT_TRUE(CheckStepMatcher(stepAndMatch.first, stepAndMatch.second));
}

TEST(CucumberStepMatcherTest, should_not_match_input_for_non_matching_steps)
{
    std::vector<std::pair<std::string, std::string>> testVectors = {
        std::make_pair("step", ""),
        std::make_pair("'%s'", "42"),
        std::make_pair("step with %s argument", "step with foo argument"),
        std::make_pair("'%s' step", "'foo step")
    };

    for (const auto& stepAndMatch : testVectors)
        EXPECT_FALSE(CheckStepMatcher(stepAndMatch.first, stepAndMatch.second));
}

TEST(CucumberStepMatcherTest, should_report_if_step_has_arguments)
{
    StepStub withArguments{ "I am here for an argument '%s'" };
    EXPECT_TRUE(withArguments.HasStringArguments());

    StepStub withoutArguments{ "Me too!" };
    EXPECT_FALSE(withoutArguments.HasStringArguments());
}

TEST(CucumberStepMatcherTest, should_report_correct_argument_count)
{
    StepStub withArguments{ "I am here for an argument '%s', %d, %b" };
    EXPECT_EQ(3, withArguments.NrArguments());

    StepStub withoutArguments{ "Me too!" };
    EXPECT_EQ(0, withoutArguments.NrArguments());
}

TEST(CucumberStepMatcherTest, get_table_argument)
{
    StepStub withTableArgument{ "I am here for a table argument" };
    infra::JsonArray arguments(R"([[["field","value"]]])");
    withTableArgument.Invoke(arguments);
    infra::BoundedString::WithStorage<5> argument;
    withTableArgument.GetTableArgument("field")->ToString(argument);
    EXPECT_EQ("value", argument);
}

TEST(CucumberStepMatcherTest, get_string_argument)
{
    StepStub withStingArgument{ "I am here for a string argument '%s'" };
    infra::JsonArray arguments(R"([ "abc" ])");
    withStingArgument.Invoke(arguments);
    infra::BoundedString::WithStorage<3> argument;
    withStingArgument.GetStringArgument(0)->ToString(argument);
    EXPECT_EQ("abc", argument);
}

TEST(CucumberStepMatcherTest, get_unsigned_integer_argument)
{
    StepStub withUIntegerArgument{ "I am here for an unsigned integer argument %d" };
    infra::JsonArray arguments(R"([ "5" ])");
    withUIntegerArgument.Invoke(arguments);
    auto argument = withUIntegerArgument.GetUIntegerArgument(0);
    EXPECT_EQ(5, *argument);
}

TEST(CucumberStepMatcherTest, get_boolean_argument)
{
    StepStub withBooleanArgument{ "I am here for an boolean argument %b and %b" };
    infra::JsonArray arguments(R"([ "true", "false" ])");
    withBooleanArgument.Invoke(arguments);
    auto argument = withBooleanArgument.GetBooleanArgument(0);
    EXPECT_EQ(true, *argument);
    argument = withBooleanArgument.GetBooleanArgument(1);
    EXPECT_EQ(false, *argument);
}

TEST(CucumberStepMatcherTest, get_all_arguments)
{
    StepStub withDifferingArguments{ "I am here for an unsigned integer argument %d, string argument '%s', boolean argument %b, and table argument" };
    infra::JsonArray arguments(R"([ "5", "abc", "true", [["field","value"]] ])");
    withDifferingArguments.Invoke(arguments);

    auto uIntegerArgument = withDifferingArguments.GetUIntegerArgument(0);
    EXPECT_EQ(5, *uIntegerArgument);

    infra::BoundedString::WithStorage<3> stringArgument;
    withDifferingArguments.GetStringArgument(1)->ToString(stringArgument);
    EXPECT_EQ("abc", stringArgument);

    auto booleanArgument = withDifferingArguments.GetBooleanArgument(2);
    EXPECT_EQ(true, *booleanArgument);

    infra::BoundedString::WithStorage<5> tableArgument;
    withDifferingArguments.GetTableArgument("field")->ToString(tableArgument);
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
