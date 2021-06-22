#include "gmock/gmock.h"
#include "infra/util/test_helper/LifetimeHelper.hpp"
#include "services/cucumber/CucumberWireProtocolParser.hpp"

class CucumberWireProtocolParserTest
    : public testing::Test
    , public infra::LifetimeHelper
{
public:
    class StepStub
        : public services::CucumberStep
    {
    public:
        using CucumberStep::CucumberStep;

        void Invoke(infra::JsonArray& arguments) final {};
    };

    StepStub aWiFiNetworkIsAvailable{ "a WiFi network is available" };
    StepStub theConnectivityNodeConnectsToThatNetwork{ "the Connectivity Node connects to that network" };
    StepStub theConnectivityNodeShouldBeConnected{ "the Connectivity Node should be connected" };
    StepStub theWiFiNetwork_IsSeenWithin_Seconds{ "the WiFi network '%s' is seen within %d seconds" };
    StepStub stepWith3Arguments{ "the WiFi network '%s' is seen within %d minutes and %d seconds" };

    services::CucumberWireProtocolParser parser;
};

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

TEST_F(CucumberWireProtocolParserTest, test_parsing_begin_scenario_tags)
{
    parser.ParseRequest(KeepAlive<infra::BoundedString>(R"(["begin_scenario",{"tags":["tag1","tag2"]}])"));
    EXPECT_EQ(parser.scenarioTags, infra::JsonObject(R"({"tags":["tag1","tag2"]})"));
}
