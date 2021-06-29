#include "gmock/gmock.h"
#include "infra/util/test_helper/LifetimeHelper.hpp"
#include "services/cucumber/CucumberWireProtocolParser.hpp"

class StepStub
    : public services::CucumberStep
{
public:
    using CucumberStep::CucumberStep;

    void Invoke(infra::JsonArray& arguments) final{};
};

bool CheckStepMatcher(infra::BoundedString stepString, infra::BoundedString matchString)
{
    StepStub step{ stepString };
    return services::CucumberStepStorage::Instance().MatchesStepName(step, matchString);
}

class CucumberWireProtocolParserTest
    : public testing::Test
    , public infra::LifetimeHelper
{
public:


    StepStub aWiFiNetworkIsAvailable{ "a WiFi network is available" };
    StepStub theConnectivityNodeConnectsToThatNetwork{ "the Connectivity Node connects to that network" };
    StepStub theConnectivityNodeShouldBeConnected{ "the Connectivity Node should be connected" };
    StepStub theWiFiNetwork_IsSeenWithin_Seconds{ "the WiFi network '%s' is seen within %d seconds" };
    StepStub stepWith3Arguments{ "the WiFi network '%s' is seen within %d minutes and %d seconds" };

    services::CucumberWireProtocolParser parser;
};

TEST(CucumberStepMatcherTest, should_match_input_for_matching_steps)
{
    std::vector<std::pair<std::string, std::string>> testVectors = {
        std::make_pair("literal step", "literal step"),
        std::make_pair("step with %d parameter", "step with 42 parameter"),
        std::make_pair("step with '%s' string argument", "step with 'foo' string argument"),
        std::make_pair("ends on parameter '%s'", "ends on parameter 'foo'"),
        std::make_pair("'%s' starts with string", "'foo' starts with string"),
        std::make_pair("'%s'", "'foo'"),
        std::make_pair("'%s' %d '%s'", "'foo' 42 'bar'"),
        std::make_pair("'%s'", "'foo bar'")
    };

    for (const auto& [stepString, matchString] : testVectors)
        EXPECT_TRUE(CheckStepMatcher(stepString, matchString));
}

TEST_F(CucumberWireProtocolParserTest, test_step_contains_arguments)
{
    EXPECT_FALSE(aWiFiNetworkIsAvailable.HasStringArguments());
    EXPECT_TRUE(theWiFiNetwork_IsSeenWithin_Seconds.HasStringArguments());
}

TEST_F(CucumberWireProtocolParserTest, test_matching_step_name)
{
    EXPECT_TRUE(services::CucumberStepStorage::Instance().MatchesStepName(aWiFiNetworkIsAvailable, KeepAlive<infra::BoundedString>("a WiFi network is available")));
    EXPECT_FALSE(services::CucumberStepStorage::Instance().MatchesStepName(stepWith3Arguments, KeepAlive<infra::BoundedString>("a WiFi network is available")));

    EXPECT_FALSE(services::CucumberStepStorage::Instance().MatchesStepName(aWiFiNetworkIsAvailable, KeepAlive<infra::BoundedString>("the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds")));
    EXPECT_TRUE(services::CucumberStepStorage::Instance().MatchesStepName(stepWith3Arguments, KeepAlive<infra::BoundedString>("the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds")));

    EXPECT_FALSE(services::CucumberStepStorage::Instance().MatchesStepName(aWiFiNetworkIsAvailable, KeepAlive<infra::BoundedString>("the WiFi network 'CoCoCo' is seen within '10' minutes and '30' seconds")));
    EXPECT_FALSE(services::CucumberStepStorage::Instance().MatchesStepName(stepWith3Arguments, KeepAlive<infra::BoundedString>("the WiFi network 'CoCoCo' is seen within '10' minutes and '30' seconds")));
}

TEST_F(CucumberWireProtocolParserTest, test_step_nr_of_arguments)
{
    EXPECT_EQ(2, theWiFiNetwork_IsSeenWithin_Seconds.NrArguments());
}

TEST_F(CucumberWireProtocolParserTest, should_parse_begin_scenario_tags)
{
    parser.ParseRequest(KeepAlive<infra::BoundedString>(R"(["begin_scenario",{"tags":["tag1","tag2"]}])"));
    EXPECT_EQ(parser.scenarioTags, infra::JsonObject(R"({"tags":["tag1","tag2"]})"));
}
