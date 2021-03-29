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
    {}

    ~CucumberWireProtocolpServerTest()
    {
        services::CucumberStepStorage::Instance().ClearStorage();
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

TEST_F(CucumberWireProtocolpServerTest, invoke_mock_step)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::StringOutputStream::WithStorage<256> inputStream;

    services::CucumberStepStorage::Instance().AddStep(cucumberStepMock);
    
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

    GIVEN("the WiFi network '%s' is seen within %d minutes and %d seconds", {
        return true;
    });

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

    GIVEN("a duplicate feature", {
        return true;
    });

    GIVEN("a duplicate feature", {
        return true;
    });

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

    GIVEN("a step", { return true; });

    infra::ConstByteRange data = infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"a step\"}]");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    infra::StringOutputStream::WithStorage<256> inputStream;

    inputStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchStep("a step")->Id() << "\", \"args\":[] } ] ]\n";
    std::vector<uint8_t> responseVector(inputStream.Storage().begin(), inputStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_step_match_request_with_arguments_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    GIVEN("the WiFi network '%s' is seen within %d minutes", {
        return true;
    });

    connection.SimulateDataReceived(infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 60 minutes\"}]"));
    ExecuteAllActions();

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes")->Id() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(inputStream.Storage().begin(), inputStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_non_matching_substring_of_step_request_with_success)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    GIVEN("the WiFi network '%s' is seen within %d minutes", {
        return true;
    });

    GIVEN("the WiFi network '%s' is seen within %d minutes and %d seconds", {
        return true;
    });

    infra::BoundedString::WithStorage<256> input = "[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 60 minutes\"}]";
    connection.SimulateDataReceived(infra::StringAsByteRange(input));
    ExecuteAllActions();

    infra::StringOutputStream::WithStorage<256> responseStream;
    responseStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchStep("the WiFi network 'CoCoCo' is seen within 60 minutes")->Id() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"60\", \"pos\":41 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_non_matching_substring_of_step_request_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    GIVEN("a WiFi network is seen within %d seconds", {
        return true;
    });

    infra::BoundedString::WithStorage<256> input = "[\"step_matches\",{\"name_to_match\":\"a WiFi network is\"}]";
    connection.SimulateDataReceived(infra::StringAsByteRange(input));
    ExecuteAllActions();

    EXPECT_EQ(nullptr, services::CucumberStepStorage::Instance().MatchStep("a WiFi network is"));

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

    GIVEN("the WiFi network '%s' is seen within %d minutes and %d seconds", {
        return true;
    });

    connection.SimulateDataReceived(infra::MakeStringByteRange("[\"step_matches\",{\"name_to_match\":\"the WiFi network 'CoCoCo' is seen within 10 minutes and 30 seconds\"}]"));
    ExecuteAllActions();

    infra::StringOutputStream::WithStorage<256> responseStream;
    responseStream << "[ \"success\", [ { \"id\":\"" << services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes and %d seconds")->Id() << "\", \"args\":[ { \"val\":\"CoCoCo\", \"pos\":18 }, { \"val\":\"10\", \"pos\":41 }, { \"val\":\"30\", \"pos\":56 } ] } ] ]\n";
    std::vector<uint8_t> responseVector(responseStream.Storage().begin(), responseStream.Storage().end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_excessive_argument_list_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    static infra::IoOutputStream ioOutputStream;
    static services::Tracer tracer(ioOutputStream);

    GIVEN("the Node connects to that network", {
        tracer.Trace() << *GetTableArgument<infra::JsonString>("invalid", 1) << "\n";
        return false;
    });

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network")->Id() << R"(","args":[[["invalid","arguments"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invoke Failed\", \"exception\":\"Exception.Invoke.Failed\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_invalid_table_header_with_fail)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    GIVEN("a network is available", {
        if (GetTableArgument<infra::JsonString>("field", 1) != nullptr)
            return true;
        else
            return false;
    });

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("a network is available")->Id() << R"(","args":[[["invalid","header"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invoke Failed\", \"exception\":\"Exception.Invoke.Failed\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_nonexistent_argument_table_with_fail_1)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    GIVEN("a network is available", {
        if (GetTableArgument<infra::JsonString>("key", 1) == nullptr)
            return false;
        else
            return true;
    })

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("a network is available")->Id() << R"(","args":[[["ssid"],["CoCoCo"],["WLAN"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invoke Failed\", \"exception\":\"Exception.Invoke.Failed\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_invalid_argument_table_with_fail_2)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    GIVEN("the Node connects to that network", {
        if (GetTableArgument<infra::JsonString>("field", 1) == nullptr)
            return false;
        else
            return true;
    })

        infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network")->Id() << R"(","args":[[["invalid","arguments"],["CoCoCo","password"],["WLAN","1234"]]]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));

    ExecuteAllActions();
    std::string response = "[ \"fail\", { \"message\":\"Invoke Failed\", \"exception\":\"Exception.Invoke.Failed\" } ]\n";
    std::vector<uint8_t> responseVector(response.begin(), response.end());
    EXPECT_EQ(responseVector, connection.sentData);

    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, should_respond_to_invoke_request_with_invalid_argument_table_with_fail_3)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    GIVEN("the Node connects to that network", {
        if (GetTableArgument<infra::JsonString>("key", 1) == nullptr)
            return false;
        else
            return true;
    })

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network")->Id() << R"(","args":[[["invalid","arguments"],["CoCoCo","password"],["WLAN","1234"]]]}])";
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

    GIVEN("the Node connects to that network", {
        for (size_t i = 0; i < NrRows(); i++)
        {
            if (GetTableArgument<infra::JsonString>("ssid", i) == nullptr)
                return false;
            else
                return true;
        }
    });

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network")->Id() << R"(","args":[[["ssid","key"],["CoCoCo","password"],["WLAN","1234"]]]}])";
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

    static infra::IoOutputStream ioOutputStream;
    static services::Tracer tracer(ioOutputStream);

    GIVEN("the WiFi network '%s' is seen within %d minutes", {
        if (ContainsStringArguments())
            for (uint8_t i = 0; i < NrStringArguments(); i++)
                if (GetStringArgument<infra::JsonString>(i) != nullptr)
                    tracer.Trace() << *GetStringArgument<infra::JsonString>(i) << "\n";
                else
                    return false;
        else
            return false;
    });

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes")->Id() << R"(","args":["HOMELAN", "10"]}])";
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

    static infra::IoOutputStream ioOutputStream;
    static services::Tracer tracer(ioOutputStream);

    GIVEN("the WiFi network '%s' is seen within %d minutes", {
        if (ContainsStringArguments())
        {
            for (uint8_t i = 0; i < NrStringArguments(); i++)
                if (GetStringArgument<infra::JsonString>(i) != nullptr)
                    tracer.Trace() << *GetStringArgument<infra::JsonString>(i) << "\n";
                else
                    return false;
            for (uint8_t i = 0; i < NrRows(); i++)
            {
                if (GetTableArgument<infra::JsonString>("field", i) != nullptr)
                    tracer.Trace() << *GetTableArgument<infra::JsonString>("field", i) << "\n";
                else
                    return false;
            }
        }
        else
            return false;
        return true;
    });

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("the WiFi network '%s' is seen within %d minutes")->Id() << R"(","args":["argument", "35", [["field","value"],["CoCoCo","password"],["WLAN","1234"]]]}])";
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

    GIVEN("the Node connects to that network", {
        return true;
    })

    infra::StringOutputStream::WithStorage<128> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("the Node connects to that network")->Id() << R"(","args":[]}])";
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

    GIVEN("the WiFi network '%s' is seen within %d minutes", {
        return true;
    });

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

    GIVEN("a WiFi network is available", {
        if (GetTableArgument<infra::JsonString>("ssid", 1) != nullptr)
            return true;
        else
        {
            *GetTableArgument<infra::JsonString>("ssid", 1);
            return false;
        }
    });

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

    GIVEN("sentence with '%s' and %d digit", {
        return true;
    });

    infra::StringOutputStream::WithStorage<256> inputStream;
    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("sentence with 'argument' and 35 digit")->Id() << R"(","args":["argument", "35", [["field","value"],["CoCoCo","password"],["WLAN","1234"]]]}])";
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
        return true;
    });

    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::StringOutputStream::WithStorage<256> inputStream;

    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("when macro is called")->Id() << R"(","args":[]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));
    ExecuteAllActions();
    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, test_2_macros)
{
    static infra::IoOutputStream ioOutputStream;
    static services::Tracer tracer(ioOutputStream);
    GIVEN("when macro is called",
    {
        tracer.Trace() << "Macro's invoke is called\n";
    return true;
    });

    GIVEN("when another macro is called", {
        tracer.Trace() << "Another Macro's invoke is called\n";
    return true;
    });

    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    infra::StringOutputStream::WithStorage<256> inputStream;

    inputStream << R"(["invoke",{"id":")" << services::CucumberStepStorage::Instance().MatchStep("when macro is called")->Id() << R"(","args":[]}])";
    connection.SimulateDataReceived(infra::StringAsByteRange(inputStream.Storage()));
    ExecuteAllActions();
    connection.sentData.clear();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}
