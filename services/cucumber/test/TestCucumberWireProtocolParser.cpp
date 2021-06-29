#include "gmock/gmock.h"
#include "services/cucumber/CucumberWireProtocolParser.hpp"

class StepStub
    : public services::CucumberStep
{
public:
    StepStub(infra::BoundedConstString stepName)
        : services::CucumberStep(stepName, "")
    {}

    void Invoke(infra::JsonArray& arguments) final {};
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
        std::make_pair("step with '%s' argument", "step with 'foo' argument"),
        std::make_pair("ends on parameter '%s'", "ends on parameter 'foo'"),
        std::make_pair("'%s' starts with string", "'foo' starts with string"),
        std::make_pair("'%s'", "'foo'"),
        std::make_pair("'%s' %d '%s'", "'foo' 42 'bar'"),
        std::make_pair("'%s'", "'foo bar'"),
        std::make_pair("", "")
    };

    for (const auto& [stepString, matchString] : testVectors)
        EXPECT_TRUE(CheckStepMatcher(stepString, matchString));
}

TEST(CucumberStepMatcherTest, should_not_match_input_for_non_matching_steps)
{
    std::vector<std::pair<std::string, std::string>> testVectors = {
        std::make_pair("step", ""),
        std::make_pair("'%s'", "42"),
        std::make_pair("'%s' step", "'foo step")
    };

    for (const auto& [stepString, matchString] : testVectors)
        EXPECT_FALSE(CheckStepMatcher(stepString, matchString));
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
    StepStub withArguments{ "I am here for an argument '%s', %d" };
    EXPECT_EQ(2, withArguments.NrArguments());

    StepStub withoutArguments{ "Me too!" };
    EXPECT_EQ(0, withoutArguments.NrArguments());
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
