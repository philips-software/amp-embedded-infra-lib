#include "gmock/gmock.h"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/cucumber/CucumberWireProtocolServer.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"

class CucumberWireProtocolReceiverTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    CucumberWireProtocolReceiverTest()
        : aWiFiNetworkIsAvailable("a WiFi network is available")
        , theConnectivityNodeConnectsToThatNetwork("the Connectivity Node connects to that network")
        , theConnectivityNodeShouldBeConnected("the Connectivity Node should be connected")
        , theWiFiNetwork_IsSeenWithin_Seconds("the WiFi network '%s' is seen within %d seconds")
        , stepWith3Arguments("the WiFi network '%s' is seen within %d minutes and %d seconds")
    {}

    class AWiFiNetworkIsAvailable : public services::CucumberStep
    {
    public:
        AWiFiNetworkIsAvailable(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        void Invoke(infra::JsonArray& arguments) {};
    };

    class TheConnectivityNodeConnectsToThatNetwork : public services::CucumberStep
    {
    public:
        TheConnectivityNodeConnectsToThatNetwork(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        void Invoke(infra::JsonArray& arguments) {};
    };

    class TheConnectivityNodeShouldBeConnected : public services::CucumberStep
    {
    public:
        TheConnectivityNodeShouldBeConnected(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        void Invoke(infra::JsonArray& arguments) {};
    };

    class TheWiFiNetwork_IsSeenWithin_Seconds : public services::CucumberStep
    {
    public:
        TheWiFiNetwork_IsSeenWithin_Seconds(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        void Invoke(infra::JsonArray& arguments) {};
    };

    class StepWith3Arguments : public services::CucumberStep
    {
    public:
        StepWith3Arguments(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        void Invoke(infra::JsonArray& arguments) {};
    };

    AWiFiNetworkIsAvailable aWiFiNetworkIsAvailable;
    TheConnectivityNodeConnectsToThatNetwork theConnectivityNodeConnectsToThatNetwork;
    TheConnectivityNodeShouldBeConnected theConnectivityNodeShouldBeConnected;
    TheWiFiNetwork_IsSeenWithin_Seconds theWiFiNetwork_IsSeenWithin_Seconds;
    StepWith3Arguments stepWith3Arguments;

    services::CucumberWireProtocolParser parser;
};

TEST_F(CucumberWireProtocolReceiverTest, test_step_contains_arguments)
{
    EXPECT_FALSE(aWiFiNetworkIsAvailable.HasStringArguments());
    EXPECT_TRUE(theWiFiNetwork_IsSeenWithin_Seconds.HasStringArguments());
}

TEST_F(CucumberWireProtocolReceiverTest, test_matching_step_name)
{
    infra::BoundedString::WithStorage<128> input = "a WiFi network is available";
    EXPECT_TRUE(services::CucumberStepStorage::Instance().MatchesStepName(aWiFiNetworkIsAvailable, input));
    EXPECT_FALSE(services::CucumberStepStorage::Instance().MatchesStepName(stepWith3Arguments, input));

    input = "the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds";
    EXPECT_FALSE(services::CucumberStepStorage::Instance().MatchesStepName(aWiFiNetworkIsAvailable, input));
    EXPECT_TRUE(services::CucumberStepStorage::Instance().MatchesStepName(stepWith3Arguments, input));

    input = "the WiFi network 'CoCoCo' is seen within '10' minutes and '30' seconds";
    EXPECT_FALSE(services::CucumberStepStorage::Instance().MatchesStepName(aWiFiNetworkIsAvailable, input));
    EXPECT_FALSE(services::CucumberStepStorage::Instance().MatchesStepName(stepWith3Arguments, input));
}

TEST_F(CucumberWireProtocolReceiverTest, test_step_nr_of_arguments)
{
    EXPECT_EQ(2, theWiFiNetwork_IsSeenWithin_Seconds.NrArguments());
}

TEST_F(CucumberWireProtocolReceiverTest, test_parsing_begin_scenario_tags)
{
    infra::BoundedString::WithStorage<128> input = R"(["begin_scenario",{"tags":["tag1","tag2"]}])";
    parser.ParseRequest(input);
    EXPECT_EQ(parser.scenarioTags, infra::JsonObject(R"({"tags":["tag1","tag2"]})"));
}
