#include "gmock/gmock.h"
#include "infra/stream/test/StreamMock.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/BoundedStringMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/HttpClientBasic.hpp"
#include "services/network/test_doubles/HttpMock.hpp"

class HttpClientBasicMock
    : public services::HttpClientBasic
{
public:
    using services::HttpClientBasic::HttpClientBasic;

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

    ~HttpClientBasicTest()
    {
        httpClient.observer = nullptr;
    }

    testing::StrictMock<services::HttpClientMock> httpClient;
    testing::StrictMock<services::HttpClientConnectorMock> httpClientConnector;
    services::HttpClientObserverFactory* httpClientObserverFactory = nullptr;
    infra::Execute execute{ [this]() { EXPECT_CALL(httpClientConnector, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&httpClientObserverFactory)); } };
    infra::BoundedString::WithStorage<64> url{ "https://hostname/path" };
    testing::StrictMock<HttpClientBasicMock> controller{ url, 443, httpClientConnector };
    testing::StrictMock<infra::MockCallback<void()>> onStopped;
};

TEST_F(HttpClientBasicTest, intermittent_error_is_reported_on_ConnectionFailed)
{
    EXPECT_CALL(controller, Error(true));
    httpClientObserverFactory->ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason::nameLookupFailed);
}

TEST_F(HttpClientBasicTest, Cancel_while_connecting_results_in_CancelConnect)
{
    EXPECT_CALL(httpClientConnector, CancelConnect(testing::Ref(*httpClientObserverFactory)));
    EXPECT_CALL(onStopped, callback());
    controller.Cancel([this]() { onStopped.callback(); });
}

TEST_F(HttpClientBasicTest, Stop_while_connected_results_in_Close)
{
    EXPECT_CALL(controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.AttachObserver(client); client->Connected(); });

    EXPECT_CALL(httpClient, Close());
    EXPECT_CALL(onStopped, callback());
    controller.Cancel([this]() { onStopped.callback(); });
}

TEST_F(HttpClientBasicTest, Stop_while_connected_stops_timeout_timer)
{
    EXPECT_CALL(controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.AttachObserver(client); client->Connected(); });

    EXPECT_CALL(httpClient, Close());
    EXPECT_CALL(onStopped, callback());
    controller.Cancel([this]() { onStopped.callback(); });

    ForwardTime(std::chrono::minutes(2));
}

TEST_F(HttpClientBasicTest, Stop_while_almost_done)
{
    EXPECT_CALL(controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.AttachObserver(client); client->Connected(); });

    EXPECT_CALL(httpClient, Close());
    httpClient.observer->BodyComplete();

    EXPECT_CALL(onStopped, callback());
    controller.Cancel([this]() { onStopped.callback(); });
}

TEST_F(HttpClientBasicTest, Stop_while_done)
{
    EXPECT_CALL(controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.AttachObserver(client); client->Connected(); });

    EXPECT_CALL(controller, Done());
    EXPECT_CALL(httpClient, Close());
    httpClient.observer->BodyComplete();
    httpClient.observer->ClosingConnection();
    httpClient.observer = nullptr;

    EXPECT_CALL(onStopped, callback());
    controller.Cancel([this]() { onStopped.callback(); });
}

TEST_F(HttpClientBasicTest, connection_times_out)
{
    EXPECT_CALL(controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.AttachObserver(client); client->Connected(); });

    EXPECT_CALL(controller, Error(true));
    EXPECT_CALL(httpClient, Close());
    ForwardTime(std::chrono::minutes(1));
}

TEST_F(HttpClientBasicTest, timer_resets_after_BodyComplete)
{
    EXPECT_CALL(controller, Established());
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { httpClient.AttachObserver(client); client->Connected(); });

    EXPECT_CALL(controller, Done());
    EXPECT_CALL(httpClient, Close());
    httpClient.observer->BodyComplete();

    ForwardTime(std::chrono::minutes(1));
}
