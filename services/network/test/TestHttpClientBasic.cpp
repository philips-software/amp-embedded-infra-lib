#include "gmock/gmock.h"
#include "infra/stream/test/StreamMock.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/BoundedStringMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/HttpClientBasic.hpp"
#include "services/network/test_doubles/HttpClientMock.hpp"

class HttpClientBasicMock
    : public services::HttpClientBasic
{
public:
    using services::HttpClientBasic::HttpClientBasic;

    void GenerateContentError()
    {
        ContentError();
    }

    MOCK_METHOD0(Established, void());
    MOCK_METHOD0(Done, void());
    MOCK_METHOD1(Error, void(bool intermittentFailure));

    MOCK_METHOD1(StatusAvailable, void(services::HttpStatusCode));
    MOCK_METHOD1(HeaderAvailable, void(services::HttpHeader));
    MOCK_METHOD1(BodyAvailable, void(infra::SharedPtr<infra::StreamReader>&& reader));
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

    testing::StrictMock<services::HttpClientConnectorMock> httpClientConnector;
    services::HttpClientObserverFactory* httpClientObserverFactory = nullptr;
    infra::Execute execute{ [this]() { EXPECT_CALL(httpClientConnector, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&httpClientObserverFactory)); } };
    infra::BoundedString::WithStorage<64> url{ "https://hostname/path" };
    infra::Optional<testing::StrictMock<HttpClientBasicMock>> controller{ infra::inPlace, url, 443, httpClientConnector };
    testing::StrictMock<infra::MockCallback<void()>> onStopped;
    testing::StrictMock<services::HttpClientMock> httpClient;
};

TEST_F(HttpClientBasicTest, intermittent_error_is_reported_on_ConnectionFailed)
{
    EXPECT_CALL(*controller, Error(true));
    httpClientObserverFactory->ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason::nameLookupFailed);
}

TEST_F(HttpClientBasicTest, Cancel_while_connecting_results_in_CancelConnect)
{
    EXPECT_CALL(httpClientConnector, CancelConnect(testing::Ref(*httpClientObserverFactory)));
    EXPECT_CALL(onStopped, callback());
    controller->Cancel([this]() { onStopped.callback(); });
}

TEST_F(HttpClientBasicTest, Stop_while_connected_results_in_Close)
{
    EXPECT_CALL(*controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.Attach(client); });

    EXPECT_CALL(httpClient, CloseConnection());
    controller->Cancel([this]() { onStopped.callback(); });

    EXPECT_CALL(onStopped, callback());
    httpClient.Detach();
    testing::Mock::VerifyAndClearExpectations(&onStopped);
}

TEST_F(HttpClientBasicTest, Stop_while_connected_does_not_invoke_Done)
{
    EXPECT_CALL(*controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.Attach(client); });

    EXPECT_CALL(httpClient, CloseConnection()).WillOnce(testing::Invoke([this]()
    {
        EXPECT_CALL(onStopped, callback());
        httpClient.Detach();
        controller = infra::none;
    }));
    controller->Cancel([this]() { onStopped.callback(); });

    testing::Mock::VerifyAndClearExpectations(&onStopped);
}

TEST_F(HttpClientBasicTest, second_Stop_while_connected_does_not_result_in_second_Close_but_adapts_callback)
{
    EXPECT_CALL(*controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.Attach(client); });

    EXPECT_CALL(httpClient, CloseConnection());
    controller->Cancel([this]() { onStopped.callback(); });

    testing::StrictMock<infra::MockCallback<void()>> onStopped2;
    controller->Cancel([&onStopped2]() { onStopped2.callback(); });

    EXPECT_CALL(onStopped2, callback());
    httpClient.Detach();
    testing::Mock::VerifyAndClearExpectations(&onStopped2);
}

TEST_F(HttpClientBasicTest, Stop_while_connected_stops_timeout_timer)
{
    EXPECT_CALL(*controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.Attach(client); });

    EXPECT_CALL(httpClient, CloseConnection());
    EXPECT_CALL(onStopped, callback());
    controller->Cancel([this]() { onStopped.callback(); });

    ForwardTime(std::chrono::minutes(2));
}

TEST_F(HttpClientBasicTest, Stop_while_almost_done)
{
    EXPECT_CALL(*controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.Attach(client); });

    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().BodyComplete();

    controller->Cancel([this]() { onStopped.callback(); });

    EXPECT_CALL(onStopped, callback());
    httpClient.Detach();
    testing::Mock::VerifyAndClearExpectations(&onStopped);
}

TEST_F(HttpClientBasicTest, Stop_while_done)
{
    EXPECT_CALL(*controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.Attach(client); });

    EXPECT_CALL(*controller, Done());
    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().BodyComplete();
    httpClient.Detach();

    EXPECT_CALL(onStopped, callback());
    controller->Cancel([this]() { onStopped.callback(); });
    testing::Mock::VerifyAndClearExpectations(&onStopped);
}

TEST_F(HttpClientBasicTest, connection_times_out)
{
    EXPECT_CALL(*controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.Attach(client); });

    EXPECT_CALL(*controller, Error(true));
    EXPECT_CALL(httpClient, CloseConnection());
    ForwardTime(std::chrono::minutes(1));
}

TEST_F(HttpClientBasicTest, timer_resets_after_BodyComplete)
{
    EXPECT_CALL(*controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.Attach(client); });

    EXPECT_CALL(*controller, Done());
    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().BodyComplete();

    ForwardTime(std::chrono::minutes(1));
}

TEST_F(HttpClientBasicTest, timer_resets_after_SendStreamAvailable)
{
    EXPECT_CALL(*controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.Attach(client); });

    ForwardTime(std::chrono::seconds(30));

    infra::StreamWriterMock writer;
    httpClient.Observer().SendStreamAvailable(infra::UnOwnedSharedPtr(writer));

    ForwardTime(std::chrono::seconds(30));

    EXPECT_CALL(*controller, Error(true));
}

TEST_F(HttpClientBasicTest, Stop_after_ClosingConnection)
{
    EXPECT_CALL(*controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.Attach(client); });

    EXPECT_CALL(*controller, Error(true));
    httpClient.Detach();

    EXPECT_CALL(onStopped, callback());
    controller->Cancel([this]() { onStopped.callback(); });
    testing::Mock::VerifyAndClearExpectations(&onStopped);
}

TEST_F(HttpClientBasicTest, ContentError_calls_stop_only_once)
{
    EXPECT_CALL(*controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.Attach(client); });

    EXPECT_CALL(*controller, Error(false));
    EXPECT_CALL(httpClient, CloseConnection());
    controller->GenerateContentError();
    controller->GenerateContentError();
}
