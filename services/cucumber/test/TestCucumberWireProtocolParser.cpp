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
        : cucumberWireProtocolParser(services::CucumberStepStorage::Instance())
    {}

    ~CucumberWireProtocolParserTest()
    {
    }

    class AWiFiNetworkIsAvailable : public services::CucumberStep
    {
    public:
        AWiFiNetworkIsAvailable(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

    public:
        bool Invoke(infra::JsonArray& arguments)
        {
            return true;
        }
    };

    class TheConnectivityNodeConnectsToThatNetwork : public services::CucumberStep
    {
    public:
        TheConnectivityNodeConnectsToThatNetwork(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

    public:
        bool Invoke(infra::JsonArray& arguments)
        {
            return true;
        }
    };

    class TheConnectivityNodeShouldBeConnected : public services::CucumberStep
    {
    public:
        TheConnectivityNodeShouldBeConnected(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

    public:
        bool Invoke(infra::JsonArray& arguments)
        {
            return true;
        }
    };

    class TheWiFiNetwork_IsSeenWithin_Seconds : public services::CucumberStep
    {
    public:
        TheWiFiNetwork_IsSeenWithin_Seconds(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

    public:
        bool Invoke(infra::JsonArray& arguments)
        {
            return true;
        }
    };

    class StepWith3Arguments : public services::CucumberStep
    {
    public:
        StepWith3Arguments(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

    public:
        bool Invoke(infra::JsonArray& arguments)
        {
            return true;
        }
    };

    AWiFiNetworkIsAvailable aWiFiNetworkIsAvailable = AWiFiNetworkIsAvailable("a WiFi network is available");
    TheConnectivityNodeConnectsToThatNetwork theConnectivityNodeConnectsToThatNetwork = TheConnectivityNodeConnectsToThatNetwork("the Connectivity Node connects to that network");
    TheConnectivityNodeShouldBeConnected theConnectivityNodeShouldBeConnected = TheConnectivityNodeShouldBeConnected("the Connectivity Node should be connected");
    TheWiFiNetwork_IsSeenWithin_Seconds theWiFiNetwork_IsSeenWithin_Seconds = TheWiFiNetwork_IsSeenWithin_Seconds("the WiFi network '%s' is seen within %d seconds");
    StepWith3Arguments stepWith3Arguments = StepWith3Arguments("the WiFi network '%s' is seen within %d minutes and %d seconds");

    services::CucumberWireProtocolParser cucumberWireProtocolParser;
};

TEST_F(CucumberWireProtocolParserTest, test_step_contains_arguments)
{
    EXPECT_FALSE(aWiFiNetworkIsAvailable.ContainsStringArguments());
    EXPECT_TRUE(theWiFiNetwork_IsSeenWithin_Seconds.ContainsStringArguments());
}

TEST_F(CucumberWireProtocolParserTest, test_step_parsing_arguments)
{
    infra::BoundedString::WithStorage<128> input = "the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds";
    infra::JsonArray expectedArguments("[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ]");
    
    infra::JsonArray jsonArray = stepWith3Arguments.ParseMatchArguments(input);

    EXPECT_EQ(jsonArray, expectedArguments);
}


TEST_F(CucumberWireProtocolParserTest, test_matching_step_name)
{
    infra::BoundedString::WithStorage<128> input = "a WiFi network is available";
    EXPECT_TRUE(services::CucumberStepStorage::Instance().CompareStepName(aWiFiNetworkIsAvailable, input));
    EXPECT_FALSE(services::CucumberStepStorage::Instance().CompareStepName(stepWith3Arguments, input));

    input = "the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds";
    EXPECT_FALSE(services::CucumberStepStorage::Instance().CompareStepName(aWiFiNetworkIsAvailable, input));
    EXPECT_TRUE(services::CucumberStepStorage::Instance().CompareStepName(stepWith3Arguments, input));

    input = "the WiFi network 'CoCoCo' is seen within '10' minutes and '30' seconds";
    EXPECT_FALSE(services::CucumberStepStorage::Instance().CompareStepName(aWiFiNetworkIsAvailable, input));
    EXPECT_FALSE(services::CucumberStepStorage::Instance().CompareStepName(stepWith3Arguments, input));
}

TEST_F(CucumberWireProtocolParserTest, test_step_nr_of_arguments)
{
    EXPECT_EQ(2, theWiFiNetwork_IsSeenWithin_Seconds.NrStringArguments());
}
