#include "gmock/gmock.h"
#include "infra/util/BoundedString.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "services/cucumber/CucumberWireProtocolServer.hpp"
#include "services/cucumber/CucumberStep.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"

class CucumberWireProtocolParserTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    CucumberWireProtocolParserTest()
        : cucumberWireProtocolParser(stepDataBase)
    {
        this->stepDataBase.AddStep(AWiFiNetworkIsAvailable);
        this->stepDataBase.AddStep(TheConnectivityNodeConnectsToThatNetwork);
        this->stepDataBase.AddStep(TheConnectivityNodeShouldBeConnected);
        this->stepDataBase.AddStep(TheWiFiNetwork_IsSeenWithin_Seconds);
        this->stepDataBase.AddStep(StepWith3Arguments);
    }

    services::StepStorage stepDataBase;

    services::Step AWiFiNetworkIsAvailable = services::Step(infra::JsonArray("[]"), infra::JsonArray("[\"ssid\", \"key\"]"), "a WiFi network is available");
    services::Step TheConnectivityNodeConnectsToThatNetwork = services::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the Connectivity Node connects to that network");
    services::Step TheConnectivityNodeShouldBeConnected = services::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the Connectivity Node should be connected");
    services::Step TheWiFiNetwork_IsSeenWithin_Seconds = services::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the WiFi network '%s' is seen within %d seconds");
    services::Step StepWith3Arguments = services::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the WiFi network '%s' is seen within %d minutes and %d seconds");

    services::CucumberWireProtocolParser cucumberWireProtocolParser;
};

TEST_F(CucumberWireProtocolParserTest, test_contains_arguments)
{
    EXPECT_FALSE(this->cucumberWireProtocolParser.ContainsArguments("a regular string"));
    EXPECT_TRUE(this->cucumberWireProtocolParser.ContainsArguments("wait 9 seconds"));
    EXPECT_TRUE(this->cucumberWireProtocolParser.ContainsArguments("a WiFi network 'CoCoCo'"));
}

TEST_F(CucumberWireProtocolParserTest, test_step_contains_arguments)
{
    EXPECT_FALSE(this->AWiFiNetworkIsAvailable.ContainsArguments());
    EXPECT_TRUE(this->TheWiFiNetwork_IsSeenWithin_Seconds.ContainsArguments());
}

TEST_F(CucumberWireProtocolParserTest, test_step_parsing_arguments)
{
    infra::BoundedString::WithStorage<128> input = "the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds";
    infra::JsonArray expectedArguments("[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ]");
    
    infra::BoundedString::WithStorage<128> arrayBuffer;
    infra::JsonArray jsonArray = this->StepWith3Arguments.ParseArguments(input, arrayBuffer);

    EXPECT_EQ(jsonArray, expectedArguments);
}

TEST_F(CucumberWireProtocolParserTest, test_matching_step_name)
{
    infra::BoundedString::WithStorage<128> input = "a WiFi network is available";
    EXPECT_TRUE(stepDataBase.CompareStepName(AWiFiNetworkIsAvailable, input));
    EXPECT_FALSE(stepDataBase.CompareStepName(StepWith3Arguments, input));

    input = "the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds";
    EXPECT_FALSE(stepDataBase.CompareStepName(AWiFiNetworkIsAvailable, input));
    EXPECT_TRUE(stepDataBase.CompareStepName(StepWith3Arguments, input));

    input = "the WiFi network 'CoCoCo' is seen within '10' minutes and '30' seconds";
    EXPECT_FALSE(stepDataBase.CompareStepName(AWiFiNetworkIsAvailable, input));
    EXPECT_FALSE(stepDataBase.CompareStepName(StepWith3Arguments, input));
}

TEST_F(CucumberWireProtocolParserTest, test_step_nr_of_arguments)
{
    EXPECT_EQ(2, TheWiFiNetwork_IsSeenWithin_Seconds.NrArguments());
}
