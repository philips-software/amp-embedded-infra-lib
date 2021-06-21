#include "gtest/gtest.h"
#include "hal/generic/TimerServiceGeneric.hpp"
#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/IoOutputStream.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/cucumber/CucumberStepMacro.hpp"
#include "services/cucumber/CucumberWireProtocolServer.hpp"
#include "services/cucumber/WiFiSetupContext.hpp"
#include "services/cucumber/test/CucumberWireProtocolServerMock.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"
#include "services/network_win/EventDispatcherWithNetwork.hpp"
#include "services/tracer/Tracer.hpp"

static services::CucumberStepStorage stepStorage;
static uint8_t val = 42;

GIVEN("a duplicate feature", infra::None)
{
    Success();
}

GIVEN("a duplicate feature", infra::None)
{
    Success();
}

GIVEN("a step", infra::None)
{
    Success();
}

GIVEN("the WiFi network '%s' is seen within %d minutes", infra::None)
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the WiFi network '%s' is seen within %d minutes and %d seconds", infra::None)
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1) && ContainsStringArgument(2))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the WiFi network '%s' is seen within %d minutes '%s' is seen within %d seconds", infra::None)
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1) && ContainsStringArgument(2) && ContainsStringArgument(3))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the Node connects to that network", infra::None)
{
    if (ContainsTableArgument("ssid"))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("a network is available", infra::None)
{
    if (ContainsTableArgument("field") && ContainsTableArgument("ssid") && ContainsTableArgument("key"))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("sentence with '%s' and %d digit", infra::None)
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1))
        if (ContainsTableArgument("field") && ContainsTableArgument("ssid") && ContainsTableArgument("key"))
            Success();
        else
            Error("Incorrect Arguments");
    GetTableArgument("field");
}

GIVEN("when macro is called", infra::None)
{
    Success();
}

GIVEN("when another macro is called", infra::None)
{
    Success();
}

GIVEN("a value is stored", infra::None)
{
    Context().Add("key0", &val);
    Success();
}

GIVEN("a value is used", infra::None)
{
    if (Context().Get<uint8_t>("key0") == 42)
        Success();
    else
        Error("Value not equal");
}

GIVEN("nothing happens for %d seconds", infra::None)
{
    if (ContainsStringArgument(0))
        if (!Context().TimeoutTimer().Armed())
        {
            infra::BoundedString::WithStorage<2> secondsString;
            GetStringArgument(0)->ToString(secondsString);
            infra::StringInputStream secondsStream(secondsString);
            uint32_t seconds;
            secondsStream >> seconds;
            Context().TimeoutTimer().Start(std::chrono::seconds(seconds), [=]() {
                Success();
            });
        }
}


class CucumberWireProtocolServerTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    CucumberWireProtocolServerTest()
        : connectionPtr(infra::UnOwnedSharedPtr(connection))
        , execute([this]() { EXPECT_CALL(connectionFactoryMock, Listen(1234, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(nullptr))); })
        , cucumberServer(connectionFactoryMock, 1234)
        , tracer(ioOutputStream)
        , context()
    {}

    ~CucumberWireProtocolServerTest()
    {
        cucumberServer.Stop(infra::emptyFunction);
    }

    infra::IoOutputStream ioOutputStream;
    services::Tracer tracer;
    testing::StrictMock<services::ConnectionStub> connection;
    infra::SharedPtr<services::ConnectionStub> connectionPtr;
    testing::StrictMock<services::ConnectionFactoryMock> connectionFactoryMock;
    services::ServerConnectionObserverFactory* serverConnectionObserverFactory;
    infra::Execute execute;
    testing::StrictMock<services::CucumberStepMock> cucumberStepMock;
    services::CucumberWireProtocolServer::WithBuffer<512> cucumberServer;
    services::CucumberContext context;
};

TEST_F(CucumberWireProtocolServerTest, accept_connection)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, accept_ipv6_connection)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv6AddressLocalHost());
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_non_json_format_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    
    infra::ConstByteRange data = infra::MakeStringByteRange("some non-json string");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();

    EXPECT_EQ("[ \"fail\", { \"message\":\"Invalid Request\", \"exception\":\"Exception.InvalidRequestType\" } ]\n", connection.SentDataAsString());

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invalid_input_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("[\"invalid_request\",{\"argument\":\"fail\"}]");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();

    EXPECT_EQ("[ \"fail\", { \"message\":\"Invalid Request\", \"exception\":\"Exception.InvalidRequestType\" } ]\n", connection.SentDataAsString());

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_step_parsing_arguments)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::ConstByteRange data = infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds\"}]");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();

    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes and %d seconds");
    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << "[ \"success\", [ { \"id\":\"" << match.id << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(inputStream.Storage().begin(), inputStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_step_parsing_arguments_2)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 10 minutes 'CoCoCo2' is seen within 30 seconds\"}]");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();

    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes '%s' is seen within %d seconds");
    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << "[ \"success\", [ { \"id\":\"" << match.id << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"CoCoCo2\", \"pos\":53 }, { \"val\":\"30\", \"pos\":77 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(inputStream.Storage().begin(), inputStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_nonexistent_step_match_request_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"we're all wired\"}]");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();

    EXPECT_EQ("[ \"fail\", { \"message\":\"Step not Matched\", \"exception\":\"Exception.Step.NotFound\" } ]\n", connection.SentDataAsString());

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_string_of_request_with_arguments_in_wrong_place_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    connection.SimulateDataReceived(infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within minutes 10 and 30 seconds\"}]"));
    ExecuteAllActions();

    EXPECT_EQ("[ \"fail\", { \"message\":\"Step not Matched\", \"exception\":\"Exception.Step.NotFound\" } ]\n", connection.SentDataAsString());

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_duplicate_step_match_request_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    connection.SimulateDataReceived(infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"a duplicate feature\"}]"));
    ExecuteAllActions();

    EXPECT_EQ("[ \"fail\", { \"message\":\"Duplicate Step\", \"exception\":\"Exception.Step.Duplicate\" } ]\n", connection.SentDataAsString());

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_step_match_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"a step\"}]");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    infra::StringOutputStream::WithStorage<256> inputStream;

    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("a step");
    inputStream << "[ \"success\", [ { \"id\":\"" << match.id << "\", \"args\":[] } ] ]\n";
    std::vector<uint8_t> responseVector(inputStream.Storage().begin(), inputStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_step_match_request_with_arguments_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    connection.SimulateDataReceived(infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 60 minutes\"}]"));
    ExecuteAllActions();

    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes");
    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << "[ \"success\", [ { \"id\":\"" << match.id << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(inputStream.Storage().begin(), inputStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_non_matching_substring_of_step_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::BoundedString::WithStorage<256> input = "[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 60 minutes\"}]";
    connection.SimulateDataReceived(infra::StringAsByteRange(input));
    ExecuteAllActions();

    infra::StringOutputStream::WithStorage<256> responseStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network 'CoCoCo' is seen within 60 minutes");
    responseStream << "[ \"success\", [ { \"id\":\"" << match.id << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_non_matching_substring_of_step_request_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::BoundedString::WithStorage<256> input = "[\"step_matches\",{\"name_to_match\":\"a WiFi network is\"}]";
    connection.SimulateDataReceived(infra::StringAsByteRange(input));
    ExecuteAllActions();
    
    EXPECT_EQ(services::CucumberStepStorage::Fail, services::CucumberStepStorage::Instance().MatchStep("a WiFi network is").result);

    infra::StringOutputStream::WithStorage<256> responseStream;
    responseStream << "[ \"fail\", { \"message\":\"Step not Matched\", \"exception\":\"Exception.Step.NotFound\" } ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_long_matching_string_of_step_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    connection.SimulateDataReceived(infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds\"}]"));
    ExecuteAllActions();

    infra::StringOutputStream::WithStorage<256> responseStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes and %d seconds");
    responseStream << "[ \"success\", [ { \"id\":\"" << match.id << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_invalid_field_table_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":[[["field","value"],["invalid","password"],["invalid","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invoke Failed\", \"exception\":\"Incorrect Arguments\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_onecollumn_argument_table_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("a network is available");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":[[["field"],["ssid"],["key"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invoke Failed\", \"exception\":\"Incorrect Arguments\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_invoke_request_table_parsing)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":[[["ssid","key"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_invoke_request_string_arguments_parsing)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":["HOMELAN", "10"]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_invoke_request_string_arguments_with_table_parsing)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":["argument", "35", [["field","value"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<128> inputStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("a step");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":[]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_arguments_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network 'HOMELAN' is seen within 10 minutes");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":["HOMELAN", "10"]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_table_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":[[["ssid","key"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_arguments_and_table_list_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("sentence with 'argument' and 35 digit");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":["argument", "35", [["field","value"],["ssid","CoCoCo"],["key","password"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_begin_scenario_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    EXPECT_CALL(connectionFactoryMock, Connect);
    testing::StrictMock<services::CucumberEchoClientMock::WithMaxConnections<1>> echoClientMock(connectionFactoryMock, services::IPv4AddressLocalHost(), tracer);
    echoClientMock.Connect();
    services::CucumberContext::Instance().Add("EchoClient", &echoClientMock);

    infra::ConstByteRange data = infra::MakeStringByteRange(R"(["begin_scenario"])");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    std::string response = "[ \"success\", [  ] ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_end_scenario_request_with_success)
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

TEST_F(CucumberWireProtocolServerTest, should_respond_to_snippet_text_request_with_snippet)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange(R"(["snippet_text"])");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();

    EXPECT_EQ("[ \"success\", \"snippet\" ]\n", connection.SentDataAsString());

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_context_storage)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("a value is stored");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":[]}])";

    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));
    ExecuteAllActions();
    connection.sentData.clear();

    EXPECT_TRUE(context.Contains("key0"));

    inputStream.Storage().clear();
    match = services::CucumberStepStorage::Instance().MatchStep("a value is used");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":[]}])";

    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));
    ExecuteAllActions();
    connection.sentData.clear();

    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_timout_timer)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::StringOutputStream::WithStorage<256> inputStream;
    services::CucumberStepStorage::Match match = services::CucumberStepStorage::Instance().MatchStep("nothing happens for 30 seconds");
    inputStream << R"(["invoke",{"id":")" << match.id << R"(","args":["30"]}])";

    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));
    ExecuteAllActions();
    EXPECT_EQ("", connection.SentDataAsString());

    ForwardTime(std::chrono::minutes(1));
    ExecuteAllActions();
    EXPECT_EQ("[ \"success\", [  ] ]\n", connection.SentDataAsString());
 
    connection.sentData.clear();

    EXPECT_CALL(connection, AbortAndDestroyMock());
}
