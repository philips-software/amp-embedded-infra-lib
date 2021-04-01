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
#include "infra/syntax/Json.hpp"

#include "services/cucumber/CucumberStepMacro.hpp"

static services::CucumberStepStorage stepStorage;

GIVEN("a duplicate feature") 
{
    return true;
}

GIVEN("a duplicate feature") 
{
    return true;
}

GIVEN("a step") 
{ 
    return true; 
}

GIVEN("the WiFi network '%s' is seen within %d minutes")
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1))
        return true;
    return false;
}

GIVEN("the WiFi network '%s' is seen within %d minutes and %d seconds")
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1) && ContainsStringArgument(2))
        return true;
    return false;
}

GIVEN("the Node connects to that network") 
{
    if (ContainsTableArgument("ssid"))
        return true;
    return false;
}

GIVEN("a network is available") 
{
    if (ContainsTableArgument("field") && ContainsTableArgument("ssid") && ContainsTableArgument("key"))
        return true;
    return false;
}

GIVEN("sentence with '%s' and %d digit")
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1))
        if (ContainsTableArgument("field") && ContainsTableArgument("ssid") && ContainsTableArgument("key"))
            return true;
    return false;
}

GIVEN("when macro is called")
{
    return true;
}

GIVEN("when another macro is called")
{
    return true;
}

class CucumberWireProtocolpServerTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    CucumberWireProtocolpServerTest()
        : connectionPtr(infra::UnOwnedSharedPtr(connection))
        , execute([this]() { EXPECT_CALL(connectionFactoryMock, Listen(1234, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(nullptr))); })
        , cucumberServer(connectionFactoryMock, 1234, services::CucumberStepStorage::Instance())
    {}

    ~CucumberWireProtocolpServerTest()
    {
        cucumberServer.Stop(infra::emptyFunction);
    }

    testing::StrictMock<services::ConnectionStub> connection;
    infra::SharedPtr<services::ConnectionStub> connectionPtr;
    testing::StrictMock<services::ConnectionFactoryMock> connectionFactoryMock;
    services::ServerConnectionObserverFactory* serverConnectionObserverFactory;
    infra::Execute execute;
    testing::StrictMock<services::CucumberStepMock> cucumberStepMock;
    services::CucumberWireProtocolServer::WithBuffer<512> cucumberServer;
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

/*
TEST_F(CucumberWireProtocolpServerTest, invoke_mock_step)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::StringOutputStream::WithStorage<256> inputStream;

    services::CucumberStepStorage::Instance().AddStep(cucumberStepMock);
    
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep(cucumberStepMock.StepName())->Id() << R"(","args":[]}])";
    EXPECT_CALL(cucumberStepMock, Invoke);
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));
    ExecuteAllActions();
    
    services::CucumberStepStorage::Instance().ClearStorage();
    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}
*/

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

    infra::ConstByteRange data = infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"a step\"}]");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    infra::StringOutputStream::WithStorage<256> inputStream;

    services::CucumberStepStorage::Instance().MatchStep("a step");
    inputStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchId() << "\", \"args\":[] } ] ]\n";
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

    services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes");
    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchId() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ] } ] ]\n";
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

    infra::StringOutputStream::WithStorage<256> responseStream;
    services::CucumberStepStorage::Instance().MatchStep("the WiFi network 'CoCoCo' is seen within 60 minutes");
    responseStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchId() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ] } ] ]\n";
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

    services::CucumberStepStorage::Instance().MatchStep("a WiFi network is");
    EXPECT_EQ(services::CucumberStepStorage::fail, services::CucumberStepStorage::Instance().MatchResult());

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

    infra::StringOutputStream::WithStorage<256> responseStream;
    services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes and %d seconds");
    responseStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchId() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_invalid_field_table_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network");
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchId() << R"(","args":[[["field","value"],["invalid","password"],["invalid","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invoke Failed\", \"exception\":\"Exception.Invoke.Failed\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_onecollumn_argument_table_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Instance().MatchStep("a network is available");
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchId() << R"(","args":[[["field"],["ssid"],["key"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invoke Failed\", \"exception\":\"Exception.Invoke.Failed\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, test_invoke_request_table_parsing)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network");
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchId() << R"(","args":[[["ssid","key"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, test_invoke_request_string_arguments_parsing)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes");
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchId() << R"(","args":["HOMELAN", "10"]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, test_invoke_request_string_arguments_with_table_parsing)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes");
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchId() << R"(","args":["argument", "35", [["field","value"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<128> inputStream;
    services::CucumberStepStorage::Instance().MatchStep("a step");
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchId() << R"(","args":[]}])";
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
    services::CucumberStepStorage::Instance().MatchStep("the WiFi network 'HOMELAN' is seen within 10 minutes");
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchId() << R"(","args":["HOMELAN", "10"]}])";
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
    services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network");
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchId() << R"(","args":[[["ssid","key"],["CoCoCo","password"],["WLAN","1234"]]]}])";
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

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Instance().MatchStep("sentence with 'argument' and 35 digit");
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchId() << R"(","args":["argument", "35", [["field","value"],["ssid","CoCoCo"],["key","password"]]]}])";
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
