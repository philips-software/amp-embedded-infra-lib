#include "gmock/gmock.h"
#include "infra/util/BoundedString.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "services/network/CucumberWireProtocolServer.hpp"
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

    services::StepStorage::Step AWiFiNetworkIsAvailable = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[\"ssid\", \"key\"]"), "a WiFi network is available");
    services::StepStorage::Step TheConnectivityNodeConnectsToThatNetwork = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the Connectivity Node connects to that network");
    services::StepStorage::Step TheConnectivityNodeShouldBeConnected = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the Connectivity Node should be connected");
    services::StepStorage::Step TheWiFiNetwork_IsSeenWithin_Seconds = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the WiFi network '%s' is seen within %d seconds");

    services::StepStorage::Step StepWith3Arguments = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the WiFi network '%s' is seen within %d minutes and %d seconds");

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

TEST_F(CucumberWireProtocolParserTest, test_reformatting_arguments)
{
    infra::BoundedString::WithStorage<128> input = "the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds";
    infra::BoundedString::WithStorage<128> arrayBuffer = "[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ]";
    infra::JsonArray jsonArray(arrayBuffer);

    infra::BoundedString::WithStorage<128> responseBuffer;
    this->cucumberWireProtocolParser.ReformatStepName(input, jsonArray, responseBuffer);
}


TEST_F(CucumberWireProtocolParserTest, test_parsing_arguments)
{
    infra::BoundedString::WithStorage<128> input = "sentence 'argument' after 10 seconds with a '2nd argument'";
    infra::JsonArray expectedJsonArray("[ { \"val\":\"argument\", \"pos\":10 }, { \"val\":\"10\", \"pos\":26 }, { \"val\":\"2nd argument\", \"pos\":45 } ]");
    infra::JsonArray jsonArray = this->cucumberWireProtocolParser.ParseArguments(input);
    infra::BoundedString::WithStorage<128> jsonArrayString;
    EXPECT_EQ(jsonArray, expectedJsonArray);

    //infra::BoundedString::WithStorage<128> parsedArguments = jsonArray.ObjectString();
   // EXPECT_EQ("[ { \"val\":\"10\", \"pos\":26 } ]", parsedArguments);

    EXPECT_EQ(input, "sentence '%s' after %d seconds with a '%s'");
}
