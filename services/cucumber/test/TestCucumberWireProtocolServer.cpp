#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/cucumber/CucumberWireProtocolServer.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"
#include "services/cucumber/test/CucumberWireProtocolServerMock.hpp"
#include "gmock/gmock.h"
#include "services/tracer/Tracer.hpp"
#include "infra/stream/IoOutputStream.hpp"

#include "services\cucumber\CucumberStepMacro.hpp"

static services::CucumberStepStorage stepStorage;

class CucumberWireProtocolpServerTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    CucumberWireProtocolpServerTest()
        : connectionPtr(infra::UnOwnedSharedPtr(connection))
        , execute([this]() { EXPECT_CALL(connectionFactoryMock, Listen(1234, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(nullptr))); })
        , cucumberServer(connectionFactoryMock, 1234, services::CucumberStepStorage::Instance())
    {
        SetupWifiSteps();
    }

    ~CucumberWireProtocolpServerTest()
    {
        services::CucumberStepStorage::Instance().ClearStorage();
        cucumberServer.Stop(infra::emptyFunction);
    }

    void SetupWifiSteps()
    {
        services::CucumberStepStorage::Instance().AddStep(aWiFiNetworkIsAvailable);
        services::CucumberStepStorage::Instance().AddStep(theConnectivityNodeConnectsToThatNetwork);
        services::CucumberStepStorage::Instance().AddStep(theConnectivityNodeShouldBeConnected);
        services::CucumberStepStorage::Instance().AddStep(theWiFiNetwork_IsSeenWithin_Minutes);
        services::CucumberStepStorage::Instance().AddStep(theWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds);
        services::CucumberStepStorage::Instance().AddStep(duplicateStep1);
        services::CucumberStepStorage::Instance().AddStep(duplicateStep2);
        services::CucumberStepStorage::Instance().AddStep(stepWithArgumentsAndTable);
        services::CucumberStepStorage::Instance().AddStep(cucumberStepMock);
    }

    testing::StrictMock<services::ConnectionStub> connection;
    infra::SharedPtr<services::ConnectionStub> connectionPtr;
    testing::StrictMock<services::ConnectionFactoryMock> connectionFactoryMock;
    services::ServerConnectionObserverFactory* serverConnectionObserverFactory;
    infra::Execute execute;
    testing::StrictMock<services::CucumberStepMock> cucumberStepMock;
    services::CucumberWireProtocolServer::WithBuffer<512> cucumberServer;

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

    class TheWiFiNetwork_IsSeenWithin_Minutes : public services::CucumberStep
    {
    public:
        TheWiFiNetwork_IsSeenWithin_Minutes(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        TheWiFiNetwork_IsSeenWithin_Minutes(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
            : CucumberStep(matchArguments, tableHeaders, stepName)
        {}

    public:
        void Invoke(infra::JsonArray& arguments)
        {

        }
    };

    class TheWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds : public services::CucumberStep
    {
    public:
        TheWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        TheWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
            : CucumberStep(matchArguments, tableHeaders, stepName)
        {}

    public:
        void Invoke(infra::JsonArray& arguments)
        {

        }
    };

    class DuplicateStep1 : public services::CucumberStep
    {
    public:
        DuplicateStep1(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        DuplicateStep1(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
            : CucumberStep(matchArguments, tableHeaders, stepName)
        {}

    public:
        void Invoke(infra::JsonArray& arguments)
        {

        }
    };

    class DuplicateStep2 : public services::CucumberStep
    {
    public:
        DuplicateStep2(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        DuplicateStep2(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
            : CucumberStep(matchArguments, tableHeaders, stepName)
        {}

    public:
        void Invoke(infra::JsonArray& arguments)
        {

        }
    };

    class StepWithArgumentsAndTable : public services::CucumberStep
    {
    public:
        StepWithArgumentsAndTable(const infra::BoundedString& stepName)
            : CucumberStep(stepName)
        {}

        StepWithArgumentsAndTable(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
            : CucumberStep(matchArguments, tableHeaders, stepName)
        {}

    public:
        void Invoke(infra::JsonArray& arguments)
        {

        }
    };

    AWiFiNetworkIsAvailable aWiFiNetworkIsAvailable = AWiFiNetworkIsAvailable(infra::JsonArray("[]"), infra::JsonArray("[\"ssid\", \"key\"]"), "a WiFi network is available");
    TheConnectivityNodeConnectsToThatNetwork theConnectivityNodeConnectsToThatNetwork = TheConnectivityNodeConnectsToThatNetwork("the Connectivity Node connects to that network");
    TheConnectivityNodeShouldBeConnected theConnectivityNodeShouldBeConnected = TheConnectivityNodeShouldBeConnected("the Connectivity Node should be connected");
    TheWiFiNetwork_IsSeenWithin_Minutes theWiFiNetwork_IsSeenWithin_Minutes = TheWiFiNetwork_IsSeenWithin_Minutes("the WiFi network '%s' is seen within %d minutes");
    TheWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds theWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds = TheWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds("the WiFi network '%s' is seen within %d minutes and %d seconds");

    DuplicateStep1 duplicateStep1 = DuplicateStep1("a duplicate feature");
    DuplicateStep2 duplicateStep2 = DuplicateStep2("a duplicate feature");
    StepWithArgumentsAndTable stepWithArgumentsAndTable = StepWithArgumentsAndTable(infra::JsonArray("[]"), infra::JsonArray("[\"field\", \"value\"]"), "sentence with '%s' and %d digit");
};

TEST_F(CucumberWireProtocolpServerTest, accept_connection)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, accept_ipv6_connection)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv6AddressLocalHost());
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, invoke_step)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::StringOutputStream::WithStorage<256> inputStream;
    
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep(cucumberStepMock.StepName())->Id() << R"(","args":[]}])";
    EXPECT_CALL(cucumberStepMock, Invoke);
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));
    ExecuteAllActions();
    
    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_non_json_format_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("some non-json string");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invalid Request\", \"exception\":\"Exception.InvalidRequestType\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invalid_input_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("[\"invalid_request\",{\"argument\":\"fail\"}]");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invalid Request\", \"exception\":\"Exception.InvalidRequestType\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_nonexistent_step_match_request_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"we're all wired\"}]");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Step not Matched\", \"exception\":\"Exception.Step.NotFound\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_string_of_request_with_arguments_in_wrong_place_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    connection.SimulateDataReceived(infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within minutes 10 and 30 seconds\"}]"));
    ExecuteAllActions();

    infra::StringOutputStream::WithStorage<256> responseStream;
    responseStream << "[ \"fail\", { \"message\":\"Step not Matched\", \"exception\":\"Exception.Step.NotFound\" } ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_duplicate_step_match_request_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    connection.SimulateDataReceived(infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"a duplicate feature\"}]"));
    ExecuteAllActions();

    infra::StringOutputStream::WithStorage<256> responseStream;
    responseStream << "[ \"fail\", { \"message\":\"Duplicate Step\", \"exception\":\"Exception.Step.Duplicate\" } ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_step_match_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"a WiFi network is available\"}]");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    infra::StringOutputStream::WithStorage<256> inputStream;
    EXPECT_EQ(this->aWiFiNetworkIsAvailable.Id(), services::CucumberStepStorage::Instance().MatchStep("a WiFi network is available")->Id());

    inputStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchStep("a WiFi network is available")->Id() << "\", \"args\":[] } ] ]\n";
    std::vector<uint8_t> responseVector(inputStream.Storage().begin(), inputStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_step_match_request_with_arguments_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    connection.SimulateDataReceived(infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 60 minutes\"}]"));
    ExecuteAllActions();

    infra::StringOutputStream::WithStorage<256> inputStream;
    EXPECT_EQ(this->theWiFiNetwork_IsSeenWithin_Minutes.Id(), services::CucumberStepStorage::Instance().MatchStep("the WiFi network 'CoCoCo' is seen within 60 minutes")->Id());

    inputStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes")->Id() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(inputStream.Storage().begin(), inputStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_non_matching_substring_of_step_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::BoundedString::WithStorage<256> input = "[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 60 minutes\"}]";
    connection.SimulateDataReceived(infra::StringAsByteRange(input));
    ExecuteAllActions();

    infra::BoundedString::WithStorage<128> nameToMatch = "the WiFi network 'CoCoCo' is seen within 60 minutes";

    EXPECT_EQ(theWiFiNetwork_IsSeenWithin_Minutes.Id(), services::CucumberStepStorage::Instance().MatchStep(nameToMatch)->Id());
    EXPECT_NE(theWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds.Id(), services::CucumberStepStorage::Instance().MatchStep(nameToMatch)->Id());

    infra::StringOutputStream::WithStorage<256> responseStream;
    responseStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchStep(nameToMatch)->Id() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_non_matching_substring_of_step_request_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::BoundedString::WithStorage<256> input = "[\"step_matches\",{\"name_to_match\":\"a WiFi network is\"}]";
    connection.SimulateDataReceived(infra::StringAsByteRange(input));
    ExecuteAllActions();

    infra::BoundedString::WithStorage<128> nameToMatch = "a WiFi network is";

    EXPECT_EQ(nullptr, services::CucumberStepStorage::Instance().MatchStep(nameToMatch));

    infra::StringOutputStream::WithStorage<256> responseStream;
    responseStream << "[ \"fail\", { \"message\":\"Step not Matched\", \"exception\":\"Exception.Step.NotFound\" } ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_long_matching_string_of_step_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    connection.SimulateDataReceived(infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds\"}]"));
    ExecuteAllActions();

    infra::BoundedString::WithStorage<128> nameToMatch = "the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds";

    EXPECT_NE(this->theWiFiNetwork_IsSeenWithin_Minutes.Id(), services::CucumberStepStorage::Instance().MatchStep(nameToMatch)->Id());
    EXPECT_EQ(this->theWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds.Id(), services::CucumberStepStorage::Instance().MatchStep(nameToMatch)->Id());

    infra::StringOutputStream::WithStorage<256> responseStream;
    responseStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchStep(nameToMatch)->Id() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_excessive_argument_list_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("the Connectivity Node connects to that network")->Id() << R"(","args":[[["invalid","arguments"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invalid Arguments\", \"exception\":\"Exception.Arguments.Invalid\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_invalid_table_header_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("a WiFi network is available")->Id() << R"(","args":[[["invalid","header"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invalid Arguments\", \"exception\":\"Exception.Arguments.Invalid\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_invalid_argument_table_with_fail_1)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("a WiFi network is available")->Id() << R"(","args":[[["ssid"],["CoCoCo"],["WLAN"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invalid Arguments\", \"exception\":\"Exception.Arguments.Invalid\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_invalid_argument_table_with_fail_2)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange(R"(["invoke",{"id":"1","args":[[["invalid","arguments"],["CoCoCo","password","password2"],["WLAN","1234"]]]}])");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invalid Arguments\", \"exception\":\"Exception.Arguments.Invalid\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_invalid_argument_table_with_fail_3)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange(R"(["invoke",{"id":"1","args":[[["invalid","arguments"],["CoCoCo"],["WLAN","1234"]]]}])");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invalid Arguments\", \"exception\":\"Exception.Arguments.Invalid\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_nonexistent_arguments_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("the Connectivity Node connects to that network")->Id() << R"(","args":["argument1", "argument2"]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invalid Arguments\", \"exception\":\"Exception.Arguments.Invalid\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::StringOutputStream::WithStorage<128> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("the Connectivity Node connects to that network")->Id() << R"(","args":[]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_arguments_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("the WiFi network 'HOMELAN' is seen within 10 minutes")->Id() << R"(","args":["HOMELAN", "10"]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_table_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("a WiFi network is available")->Id() << R"(","args":[[["ssid","key"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_arguments_and_table_list_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::BoundedString::WithStorage<128> nameToMatch = "sentence with 'argument' and 35 digit";

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep(nameToMatch)->Id() << R"(","args":["argument", "35", [["field","value"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_begin_scenario_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange(R"(["begin_scenario"])");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_end_scenario_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange(R"(["end_scenario"])");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_snippet_text_request_with_snippet)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange(R"(["snippet_text"])");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    std::string response = "[ \"success\", \"snippet\" ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, test_macro)
{
    static infra::IoOutputStream ioOutputStream;
    static services::Tracer tracer(ioOutputStream);
    GIVEN("when macro is called", 
    {
        tracer.Trace() << "Macro's invoke is called\n";
    });

    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::StringOutputStream::WithStorage<256> inputStream;

    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("when macro is called")->Id() << R"(","args":[]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));
    ExecuteAllActions();
    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
    
}