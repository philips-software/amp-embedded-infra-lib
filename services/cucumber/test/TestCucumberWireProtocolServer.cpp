#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/cucumber/CucumberStepMacro.hpp"
#include "services/cucumber/CucumberWireProtocolServer.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"
#include "gtest/gtest.h"

static services::CucumberStepStorage stepStorage;

GIVEN("a duplicate feature")
{
    Success();
}

GIVEN("a duplicate feature")
{
    Success();
}

GIVEN("a step")
{
    Success();
}

GIVEN("the WiFi network '%s' is seen within %d minutes")
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the WiFi network '%s' is seen within %d minutes and %d seconds")
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1) && ContainsStringArgument(2))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the WiFi network '%s' is seen within %d minutes '%s' is seen within %d seconds")
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1) && ContainsStringArgument(2) && ContainsStringArgument(3))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("the Node connects to that network")
{
    if (ContainsTableArgument("ssid") && ContainsTableArgument("foobar") && ContainsTableArgument("WLAN"))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("a network is available")
{
    if (ContainsTableArgument("field") && ContainsTableArgument("ssid") && ContainsTableArgument("key"))
        Success();
    else
        Error("Incorrect Arguments");
}

GIVEN("sentence with '%s' and %d digit")
{
    if (ContainsStringArgument(0) && ContainsStringArgument(1))
        if (ContainsTableArgument("field") && ContainsTableArgument("ssid") && ContainsTableArgument("key"))
            Success();
        else
            Error("Incorrect Arguments");
    else
        Error("Incorrect Arguments");
}

GIVEN("nothing happens for %d seconds")
{
    if (ContainsStringArgument(0))
    {
        infra::BoundedString::WithStorage<2> secondsString;
        GetStringArgument(0)->ToString(secondsString);
        infra::StringInputStream secondsStream(secondsString);
        uint32_t seconds;
        secondsStream >> seconds;
        Context().TimeoutTimer().Start(std::chrono::seconds(seconds), [this]()
            {
                Success();
            });
    }
}

class CucumberStepMock
    : public services::CucumberStepArguments
{
public:
    CucumberStepMock()
        : CucumberStepArguments("Mock Step", "")
    {}

    MOCK_METHOD1(Invoke, void(infra::JsonArray& arguments));

    void Execute() override
    {}
};

class CucumberWireProtocolServerTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    CucumberWireProtocolServerTest()
        : execute([this]()
              {
                  EXPECT_CALL(connectionFactoryMock, Listen(1234, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(nullptr)));
              })
        , cucumberServer(connectionFactoryMock, 1234, scenarioHandler)
    {}

    ~CucumberWireProtocolServerTest() override
    {
        cucumberServer.Stop(infra::emptyFunction);
    }

    void ReceiveData(infra::BoundedConstString input)
    {
        connection.SimulateDataReceived(infra::StringAsByteRange(input));
        ExecuteAllActions();
    }

    void CheckFailResponse(infra::BoundedConstString message, infra::BoundedConstString exception) const
    {
        std::string response = std::string("[ \"fail\", { \"message\":\"") + message +
                               std::string("\", \"exception\":\"") + exception + std::string("\" } ]\n");
        EXPECT_EQ(response, connection.SentDataAsString());
    }

    void CheckSuccessResponse() const
    {
        EXPECT_EQ("[ \"success\", [  ] ]\n", connection.SentDataAsString());
    }

    void CheckSuccessResponse(int id, infra::BoundedConstString arguments, infra::BoundedConstString location) const
    {
        std::string response = std::string("[ \"success\", [ { \"id\":\"") + std::to_string(id) + std::string("\", \"args\":") + arguments + std::string(", \"source\":\"") + location + std::string("\" } ] ]\n");
        EXPECT_EQ(response, connection.SentDataAsString());
    }

    void InvokeStep(int id, infra::BoundedConstString arguments)
    {
        std::string request = std::string("[\"invoke\",{\"id\":\"") + std::to_string(id) + std::string("\",\"args\":") + arguments + std::string("}]");
        ReceiveData(request);
    }

    testing::StrictMock<services::ConnectionStub> connection;
    infra::SharedPtr<services::ConnectionStub> connectionPtr{ infra::UnOwnedSharedPtr(connection) };
    testing::StrictMock<services::ConnectionFactoryMock> connectionFactoryMock;
    services::ServerConnectionObserverFactory* serverConnectionObserverFactory = nullptr;
    infra::Execute execute;
    testing::StrictMock<CucumberStepMock> cucumberStepMock;
    services::CucumberScenarioRequestHandler scenarioHandler;
    services::CucumberContext context;
    services::CucumberWireProtocolServer::WithBuffer<512> cucumberServer;
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
    ReceiveData("some non-json string");

    CheckFailResponse("Invalid Request", "Exception.InvalidRequestType");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invalid_input_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData("[\"invalid_request\",{\"argument\":\"fail\"}]");

    CheckFailResponse("Invalid Request", "Exception.InvalidRequestType");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_step_parsing_arguments)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'foobar' is seen within 10 minutes and 30 seconds\"}]");

    auto match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes and %d seconds");
    CheckSuccessResponse(match.id, "[ { \"val\":\"foobar\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ]", "TestCucumberWireProtocolServer.cpp:37");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_step_parsing_arguments_2)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'foobar' is seen within 10 minutes 'foobar2' is seen within 30 seconds\"}]");

    auto match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes '%s' is seen within %d seconds");
    CheckSuccessResponse(match.id, "[ { \"val\":\"foobar\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"foobar2\", \"pos\":53 }, { \"val\":\"30\", \"pos\":77 } ]", "TestCucumberWireProtocolServer.cpp:45");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_nonexistent_step_match_request_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData("[\"step_matches\",{\"name_to_match\":\"we're all wired\"}]");

    CheckFailResponse("Step not Matched", "Exception.Step.NotFound");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_request_with_arguments_in_wrong_place_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'foobar' is seen within minutes 10 and 30 seconds\"}]");

    CheckFailResponse("Step not Matched", "Exception.Step.NotFound");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_duplicate_step_match_request_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData("[\"step_matches\",{\"name_to_match\":\"a duplicate feature\"}]");

    CheckFailResponse("Duplicate Step", "Exception.Step.Duplicate");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_step_match_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData("[\"step_matches\",{\"name_to_match\":\"a step\"}]");

    auto match = services::CucumberStepStorage::Instance().MatchStep("a step");
    CheckSuccessResponse(match.id, "[]", "TestCucumberWireProtocolServer.cpp:24");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_step_match_request_with_arguments_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'foobar' is seen within 60 minutes\"}]");

    auto match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes");
    CheckSuccessResponse(match.id, "[ { \"val\":\"foobar\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ]", "TestCucumberWireProtocolServer.cpp:29");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_non_matching_substring_of_step_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'foobar' is seen within 60 minutes\"}]");

    auto match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network 'foobar' is seen within 60 minutes");
    CheckSuccessResponse(match.id, "[ { \"val\":\"foobar\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ]", "TestCucumberWireProtocolServer.cpp:29");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_non_matching_substring_of_step_request_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData("[\"step_matches\",{\"name_to_match\":\"a WiFi network is\"}]");

    CheckFailResponse("Step not Matched", "Exception.Step.NotFound");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_long_matching_string_of_step_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'foobar' is seen within 10 minutes and 30 seconds\"}]");

    auto match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes and %d seconds");
    CheckSuccessResponse(match.id, "[ { \"val\":\"foobar\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ]", "TestCucumberWireProtocolServer.cpp:37");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_invalid_field_table_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    auto match = services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network");
    InvokeStep(match.id, "[[[\" field \",\" value \"],[\" invalid \",\" password \"],[\" invalid \",\" 1234 \"]]]");

    CheckFailResponse("Invoke Failed", "Incorrect Arguments");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_onecollumn_argument_table_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    auto match = services::CucumberStepStorage::Instance().MatchStep("a network is available");
    InvokeStep(match.id, R"([[["field"],["ssid"],["key"]]])");

    CheckFailResponse("Invoke Failed", "Incorrect Arguments");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_invoke_request_table_parsing)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    auto match = services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network");
    InvokeStep(match.id, R"([[["ssid","key"],["foobar","password"],["WLAN","1234"]]])");

    CheckSuccessResponse();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_invoke_request_string_arguments_parsing)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    auto match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes");
    InvokeStep(match.id, R"(["HOMELAN", "10"])");

    CheckSuccessResponse();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_invoke_request_string_arguments_with_table_parsing)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    auto match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes");
    InvokeStep(match.id, R"(["argument", "35", [["field","value"],["foobar","password"],["WLAN","1234"]]])");

    CheckSuccessResponse();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    auto match = services::CucumberStepStorage::Instance().MatchStep("a step");
    InvokeStep(match.id, "[]");

    CheckSuccessResponse();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_arguments_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    auto match = services::CucumberStepStorage::Instance().MatchStep("the WiFi network 'HOMELAN' is seen within 10 minutes");
    InvokeStep(match.id, R"(["HOMELAN", "10"])");

    CheckSuccessResponse();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_table_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    auto match = services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network");
    InvokeStep(match.id, R"([[["ssid","key"],["foobar","password"],["WLAN","1234"]]])");

    CheckSuccessResponse();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_invoke_request_with_arguments_and_table_list_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    auto match = services::CucumberStepStorage::Instance().MatchStep("sentence with 'argument' and 35 digit");
    InvokeStep(match.id, R"(["argument", "35", [["field","value"],["ssid","foobar"],["key","password"]]])");

    CheckSuccessResponse();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_begin_scenario_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData(R"(["begin_scenario"])");

    CheckSuccessResponse();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_end_scenario_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData(R"(["end_scenario"])");

    CheckSuccessResponse();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, should_respond_to_snippet_text_request_with_snippet)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    ReceiveData(R"(["snippet_text"])");

    EXPECT_EQ("[ \"success\", \"snippet\" ]\n", connection.SentDataAsString());
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolServerTest, test_timout_timer)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    auto match = services::CucumberStepStorage::Instance().MatchStep("nothing happens for 30 seconds");
    InvokeStep(match.id, "[\"30\"]");
    EXPECT_EQ("", connection.SentDataAsString());

    ForwardTime(std::chrono::minutes(1));
    ExecuteAllActions();
    CheckSuccessResponse();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}
