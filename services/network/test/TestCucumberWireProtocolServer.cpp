#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/CucumberWireProtocolServer.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"
#include "gmock/gmock.h"

class CucumberWireProtocolpServerTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    CucumberWireProtocolpServerTest()
        : connectionPtr(infra::UnOwnedSharedPtr(connection))
        , execute([this]() { EXPECT_CALL(connectionFactoryMock, Listen(1234, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(nullptr))); })
        , cucumberServer(connectionFactoryMock, 1234, stepDataBase)
    {
        SetupWifiSteps();
    }

    ~CucumberWireProtocolpServerTest()
    {
        cucumberServer.Stop(infra::emptyFunction);
    }

    void SetupWifiSteps()
    {
        this->stepDataBase.AddStep(AWiFiNetworkIsAvailable);
        this->stepDataBase.AddStep(TheConnectivityNodeConnectsToThatNetwork);
        this->stepDataBase.AddStep(TheConnectivityNodeShouldBeConnected);
        this->stepDataBase.AddStep(TheWiFiNetwork_IsSeenWithin_Minutes);
        this->stepDataBase.AddStep(TheWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds);
        this->stepDataBase.AddStep(DuplicateStep1);
        this->stepDataBase.AddStep(DuplicateStep2);
    }

    testing::StrictMock<services::ConnectionStub> connection;
    infra::SharedPtr<services::ConnectionStub> connectionPtr;
    testing::StrictMock<services::ConnectionFactoryMock> connectionFactoryMock;
    services::ServerConnectionObserverFactory* serverConnectionObserverFactory;
    infra::Execute execute;
    services::CucumberWireProtocolServer::WithBuffer<512> cucumberServer;
    services::StepStorage stepDataBase;

    services::StepStorage::Step AWiFiNetworkIsAvailable = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[\"ssid\", \"key\"]"), "a WiFi network is available");
    services::StepStorage::Step TheConnectivityNodeConnectsToThatNetwork = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the Connectivity Node connects to that network");
    services::StepStorage::Step TheConnectivityNodeShouldBeConnected = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the Connectivity Node should be connected");
    services::StepStorage::Step TheWiFiNetwork_IsSeenWithin_Minutes = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the WiFi network '%s' is seen within %d minutes");
    services::StepStorage::Step TheWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "the WiFi network '%s' is seen within %d minutes and %d seconds");

    services::StepStorage::Step DuplicateStep1 = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "a duplicate feature");
    services::StepStorage::Step DuplicateStep2 = services::StepStorage::Step(infra::JsonArray("[]"), infra::JsonArray("[]"), "a duplicate feature");
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

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_invalid_argument_list_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange(R"(["invoke",{"id":"1","args":[[["invalid","arguments"],["CoCoCo","password"],["WLAN","1234"]]]}])");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invalid Arguments\", \"exception\":\"Exception.Arguments.Invalid\" } ]\n";
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

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::StringOutputStream::WithStorage<128> inputStream;
    inputStream << R"(["invoke",{"id":")" << this->stepDataBase.MatchStep("a WiFi network is available")->Id() << R"(","args":[]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_argument_list_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << this->stepDataBase.MatchStep("a WiFi network is available")->Id() << R"(","args":[[["ssid","key"],["CoCoCo","password"],["WLAN","1234"]]]}])";
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

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_step_match_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"a WiFi network is available\"}]");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    infra::StringOutputStream::WithStorage<256> inputStream;
    EXPECT_EQ(this->AWiFiNetworkIsAvailable.Id(), this->stepDataBase.MatchStep("a WiFi network is available")->Id());

    inputStream << "[ \"success\", [ { \"id\":\"" << this->stepDataBase.MatchStep("a WiFi network is available")->Id() << "\", \"args\":[] } ] ]\n";
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
    EXPECT_EQ(this->TheWiFiNetwork_IsSeenWithin_Minutes.Id(), this->stepDataBase.MatchStep("the WiFi network 'CoCoCo' is seen within 60 minutes")->Id());

    inputStream << "[ \"success\", [ { \"id\":\"" << this->stepDataBase.MatchStep("the WiFi network '%s' is seen within %d minutes")->Id() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(inputStream.Storage().begin(), inputStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_non_matching_substring_of_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::BoundedString::WithStorage<256> input = "[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 60 minutes\"}]";
    connection.SimulateDataReceived(infra::StringAsByteRange(input));
    ExecuteAllActions();

    infra::BoundedString::WithStorage<128> nameToMatch = "the WiFi network 'CoCoCo' is seen within 60 minutes";

    EXPECT_EQ(this->TheWiFiNetwork_IsSeenWithin_Minutes.Id(), this->stepDataBase.MatchStep(nameToMatch)->Id());
    EXPECT_NE(this->TheWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds.Id(), this->stepDataBase.MatchStep(nameToMatch)->Id());

    infra::StringOutputStream::WithStorage<256> responseStream;
    responseStream << "[ \"success\", [ { \"id\":\"" << this->stepDataBase.MatchStep(nameToMatch)->Id() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_long_matching_string_of_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    connection.SimulateDataReceived(infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds\"}]"));
    ExecuteAllActions();

    infra::BoundedString::WithStorage<128> nameToMatch = "the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds";

    EXPECT_NE(this->TheWiFiNetwork_IsSeenWithin_Minutes.Id(), this->stepDataBase.MatchStep(nameToMatch)->Id());
    EXPECT_EQ(this->TheWiFiNetwork_IsSeenWithin_MinutesAnd_Seconds.Id(), this->stepDataBase.MatchStep(nameToMatch)->Id());

    infra::StringOutputStream::WithStorage<256> responseStream;
    responseStream << "[ \"success\", [ { \"id\":\"" << this->stepDataBase.MatchStep(nameToMatch)->Id() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}


