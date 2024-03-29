#include "infra/stream/test/StreamMock.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/HttpClientBasic.hpp"
#include "services/network/test_doubles/HttpClientMock.hpp"
#include "gmock/gmock.h"

class HttpClientBasicMock
    : public services::HttpClientBasic
{
public:
    virtual ~HttpClientBasicMock() = default;

    using services::HttpClientBasic::HttpClientBasic;

    using services::HttpClientBasic::ContentError;
    using services::HttpClientBasic::Path;

    MOCK_METHOD(void, Established, (), (override));
    MOCK_METHOD(void, Done, (), (override));
    MOCK_METHOD(void, Error, (bool intermittentFailure), (override));

    void StatusAvailable(services::HttpStatusCode code) override
    {
        services::HttpClientBasic::StatusAvailable(code);
    }

    MOCK_METHOD(void, HeaderAvailable, (services::HttpHeader), (override));
    MOCK_METHOD(void, BodyAvailable, (infra::SharedPtr<infra::StreamReader> && reader), (override));
};

class HttpClientBasicTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    HttpClientBasicTest()
    {
        EXPECT_EQ("hostname", httpClientObserverFactory->Hostname());
        EXPECT_EQ(443, httpClientObserverFactory->Port());
    }

    void EstablishConnection()
    {
        EXPECT_CALL(*controller, Established());
        httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client)
            {
                httpClient.Attach(client);
            });
    }

    testing::StrictMock<services::HttpClientConnectorMock> httpClientConnector;
    services::HttpClientObserverFactory* httpClientObserverFactory = nullptr;
    infra::Execute execute{ [this]()
        {
            EXPECT_CALL(httpClientConnector, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&httpClientObserverFactory));
        } };
    infra::BoundedString::WithStorage<64> url{ "https://hostname/path" };
    infra::Optional<testing::StrictMock<HttpClientBasicMock>> controller{ infra::inPlace, url, 443, httpClientConnector };
    testing::StrictMock<infra::MockCallback<void()>> onStopped;
    testing::StrictMock<services::HttpClientMock> httpClient;
};

TEST_F(HttpClientBasicTest, returned_path_split_correctly)
{
    auto path = controller->Path();
    infra::BoundedConstString::WithStorage<5> result = "/path";
    EXPECT_EQ(path, result);
}

TEST_F(HttpClientBasicTest, returned_path_memory_trimmed_correctly)
{
    std::string_view hostnamePart = "https://hostname";
    auto path = controller->Path();
    EXPECT_EQ(path.end(), url.end());
    EXPECT_EQ(path.begin(), url.begin() + hostnamePart.size());
    EXPECT_EQ(path.max_size(), url.max_size() - hostnamePart.size());
}

TEST_F(HttpClientBasicTest, intermittent_error_is_reported_on_ConnectionFailed)
{
    EXPECT_CALL(*controller, Error(true));
    httpClientObserverFactory->ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason::nameLookupFailed);
}

TEST_F(HttpClientBasicTest, Cancel_while_connecting_results_in_CancelConnect)
{
    EXPECT_CALL(httpClientConnector, CancelConnect(testing::Ref(*httpClientObserverFactory)));
    EXPECT_CALL(onStopped, callback());
    controller->Cancel([this]()
        {
            onStopped.callback();
        });
}

TEST_F(HttpClientBasicTest, Stop_while_connected_results_in_Close)
{
    EstablishConnection();

    EXPECT_CALL(httpClient, CloseConnection());
    controller->Cancel([this]()
        {
            onStopped.callback();
        });

    EXPECT_CALL(onStopped, callback());
    httpClient.Detach();
    testing::Mock::VerifyAndClearExpectations(&onStopped);
}

TEST_F(HttpClientBasicTest, Stop_while_connected_does_not_invoke_Done)
{
    EstablishConnection();

    EXPECT_CALL(httpClient, CloseConnection()).WillOnce(testing::Invoke([this]()
        {
            EXPECT_CALL(onStopped, callback());
            httpClient.Detach();
            controller = infra::none;
        }));
    controller->Cancel([this]()
        {
            onStopped.callback();
        });

    testing::Mock::VerifyAndClearExpectations(&onStopped);
}

TEST_F(HttpClientBasicTest, second_Stop_while_connected_does_not_result_in_second_Close_but_adapts_callback)
{
    EstablishConnection();

    EXPECT_CALL(httpClient, CloseConnection());
    controller->Cancel([this]()
        {
            onStopped.callback();
        });

    testing::StrictMock<infra::MockCallback<void()>> onStopped2;
    controller->Cancel([&onStopped2]()
        {
            onStopped2.callback();
        });

    EXPECT_CALL(onStopped2, callback());
    httpClient.Detach();
    testing::Mock::VerifyAndClearExpectations(&onStopped2);
}

TEST_F(HttpClientBasicTest, Stop_while_connected_stops_timeout_timer)
{
    EstablishConnection();

    EXPECT_CALL(httpClient, CloseConnection());
    EXPECT_CALL(onStopped, callback());
    controller->Cancel([this]()
        {
            onStopped.callback();
        });

    ForwardTime(std::chrono::minutes(2));
}

TEST_F(HttpClientBasicTest, Stop_while_almost_done)
{
    EstablishConnection();

    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().StatusAvailable(services::HttpStatusCode::OK);
    httpClient.Observer().BodyComplete();

    controller->Cancel([this]()
        {
            onStopped.callback();
        });

    EXPECT_CALL(onStopped, callback());
    httpClient.Detach();
    testing::Mock::VerifyAndClearExpectations(&onStopped);
}

TEST_F(HttpClientBasicTest, Stop_while_done)
{
    EstablishConnection();

    EXPECT_CALL(*controller, Done());
    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().StatusAvailable(services::HttpStatusCode::OK);
    httpClient.Observer().BodyComplete();
    httpClient.Detach();

    EXPECT_CALL(onStopped, callback());
    controller->Cancel([this]()
        {
            onStopped.callback();
        });
    testing::Mock::VerifyAndClearExpectations(&onStopped);
}

TEST_F(HttpClientBasicTest, connection_times_out)
{
    EstablishConnection();

    EXPECT_CALL(*controller, Error(true));
    EXPECT_CALL(httpClient, CloseConnection());
    ForwardTime(std::chrono::minutes(1));
}

TEST_F(HttpClientBasicTest, timer_resets_after_BodyComplete)
{
    EstablishConnection();

    EXPECT_CALL(*controller, Done());
    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().StatusAvailable(services::HttpStatusCode::OK);
    httpClient.Observer().BodyComplete();

    ForwardTime(std::chrono::minutes(1));
}

TEST_F(HttpClientBasicTest, timer_resets_after_SendStreamAvailable)
{
    EstablishConnection();

    ForwardTime(std::chrono::seconds(30));

    infra::StreamWriterMock writer;
    httpClient.Observer().SendStreamAvailable(infra::UnOwnedSharedPtr(writer));

    ForwardTime(std::chrono::seconds(30));

    EXPECT_CALL(*controller, Error(true));
}

TEST_F(HttpClientBasicTest, Stop_after_ClosingConnection)
{
    EstablishConnection();

    EXPECT_CALL(*controller, Error(true));
    httpClient.Detach();

    EXPECT_CALL(onStopped, callback());
    controller->Cancel([this]()
        {
            onStopped.callback();
        });
    testing::Mock::VerifyAndClearExpectations(&onStopped);
}

TEST_F(HttpClientBasicTest, ContentError_calls_stop_only_once)
{
    EstablishConnection();

    EXPECT_CALL(*controller, Error(false));
    EXPECT_CALL(httpClient, CloseConnection());
    controller->ContentError();
    controller->ContentError();
}

TEST_F(HttpClientBasicTest, done_called_when_connection_is_reestablished)
{
    EstablishConnection();

    infra::StreamWriterMock writer;
    httpClient.Observer().SendStreamAvailable(infra::UnOwnedSharedPtr(writer));

    EXPECT_CALL(*controller, Error(true));
    httpClient.Detach();

    EstablishConnection();

    httpClient.Observer().SendStreamAvailable(infra::UnOwnedSharedPtr(writer));

    EXPECT_CALL(*controller, Done());
    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().StatusAvailable(services::HttpStatusCode::OK);
    httpClient.Observer().BodyComplete();

    ForwardTime(std::chrono::minutes(1));
}

TEST_F(HttpClientBasicTest, ContentError_Cleared_After_ReportingError)
{
    EstablishConnection();

    EXPECT_CALL(*controller, Error(false));
    EXPECT_CALL(httpClient, CloseConnection());
    controller->ContentError();
    httpClient.Detach();

    EstablishConnection();

    EXPECT_CALL(*controller, Done());
    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().StatusAvailable(services::HttpStatusCode::OK);
    httpClient.Observer().BodyComplete();
}
