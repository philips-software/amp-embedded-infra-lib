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
        this->stepDataBase.AddStep(aWiFiNetworkIsAvailable);
        this->stepDataBase.AddStep(theConnectivityNodeConnectsToThatNetwork);
        this->stepDataBase.AddStep(theConnectivityNodeShouldBeConnected);
        this->stepDataBase.AddStep(theWiFiNetwork_IsSeenWithin_Seconds);
        this->stepDataBase.AddStep(stepWith3Arguments);
    }

    services::StepStorage stepDataBase;

    class AWiFiNetworkIsAvailable : public services::CucumberStep
    {
    public:
        AWiFiNetworkIsAvailable(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        AWiFiNetworkIsAvailable(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
            : CucumberStep(matchArguments, tableHeaders, stepName)
        {}

    public:
        void Invoke(infra::JsonArray& arguments)
        {

        }
    };

    class TheConnectivityNodeConnectsToThatNetwork : public services::CucumberStep
    {
    public:
        TheConnectivityNodeConnectsToThatNetwork(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        TheConnectivityNodeConnectsToThatNetwork(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
            : CucumberStep(matchArguments, tableHeaders, stepName)
        {}

    public:
        void Invoke(infra::JsonArray& arguments)
        {

        }
    };

    class TheConnectivityNodeShouldBeConnected : public services::CucumberStep
    {
    public:
        TheConnectivityNodeShouldBeConnected(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        TheConnectivityNodeShouldBeConnected(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
            : CucumberStep(matchArguments, tableHeaders, stepName)
        {}

    public:
        void Invoke(infra::JsonArray& arguments)
        {

        }
    };

    class TheWiFiNetwork_IsSeenWithin_Seconds : public services::CucumberStep
    {
    public:
        TheWiFiNetwork_IsSeenWithin_Seconds(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        TheWiFiNetwork_IsSeenWithin_Seconds(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
            : CucumberStep(matchArguments, tableHeaders, stepName)
        {}

    public:
        void Invoke(infra::JsonArray& arguments)
        {

        }
    };

    class StepWith3Arguments : public services::CucumberStep
    {
    public:
        StepWith3Arguments(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        StepWith3Arguments(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
            : CucumberStep(matchArguments, tableHeaders, stepName)
        {}

    public:
        void Invoke(infra::JsonArray& arguments)
        {

        }
    };

    AWiFiNetworkIsAvailable aWiFiNetworkIsAvailable = AWiFiNetworkIsAvailable(infra::JsonArray("[]"), infra::JsonArray("[\"ssid\", \"key\"]"), "a WiFi network is available");
    TheConnectivityNodeConnectsToThatNetwork theConnectivityNodeConnectsToThatNetwork = TheConnectivityNodeConnectsToThatNetwork("the Connectivity Node connects to that network");
    TheConnectivityNodeShouldBeConnected theConnectivityNodeShouldBeConnected = TheConnectivityNodeShouldBeConnected(infra::JsonArray("[]"), infra::JsonArray("[]"), "the Connectivity Node should be connected");
    TheWiFiNetwork_IsSeenWithin_Seconds theWiFiNetwork_IsSeenWithin_Seconds = TheWiFiNetwork_IsSeenWithin_Seconds(infra::JsonArray("[]"), infra::JsonArray("[]"), "the WiFi network '%s' is seen within %d seconds");
    StepWith3Arguments stepWith3Arguments = StepWith3Arguments(infra::JsonArray("[]"), infra::JsonArray("[]"), "the WiFi network '%s' is seen within %d minutes and %d seconds");

    services::CucumberWireProtocolParser cucumberWireProtocolParser;
};

TEST_F(CucumberWireProtocolParserTest, test_step_contains_arguments)
{
    EXPECT_FALSE(this->aWiFiNetworkIsAvailable.ContainsArguments());
    EXPECT_TRUE(this->theWiFiNetwork_IsSeenWithin_Seconds.ContainsArguments());
}

TEST_F(CucumberWireProtocolParserTest, test_step_parsing_arguments)
{
    infra::BoundedString::WithStorage<128> input = "the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds";
    infra::JsonArray expectedArguments("[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ]");
    
    infra::BoundedString::WithStorage<128> arrayBuffer;
    infra::JsonArray jsonArray = this->stepWith3Arguments.ParseArguments(input, arrayBuffer);

    EXPECT_EQ(jsonArray, expectedArguments);
}

TEST_F(CucumberWireProtocolParserTest, test_matching_step_name)
{
    
    infra::BoundedString::WithStorage<128> input = "a WiFi network is available";
    EXPECT_TRUE(stepDataBase.CompareStepName(aWiFiNetworkIsAvailable, input));
    EXPECT_FALSE(stepDataBase.CompareStepName(stepWith3Arguments, input));

    input = "the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds";
    EXPECT_FALSE(stepDataBase.CompareStepName(aWiFiNetworkIsAvailable, input));
    EXPECT_TRUE(stepDataBase.CompareStepName(stepWith3Arguments, input));

    input = "the WiFi network 'CoCoCo' is seen within '10' minutes and '30' seconds";
    EXPECT_FALSE(stepDataBase.CompareStepName(aWiFiNetworkIsAvailable, input));
    EXPECT_FALSE(stepDataBase.CompareStepName(stepWith3Arguments, input));
    
}

TEST_F(CucumberWireProtocolParserTest, test_step_nr_of_arguments)
{
    EXPECT_EQ(2, theWiFiNetwork_IsSeenWithin_Seconds.NrArguments());
}
