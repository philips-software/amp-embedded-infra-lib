#include "infra/util/BoundedString.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/cucumber/CucumberContext.hpp"
#include "services/cucumber/CucumberStepMacro.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/HttpClientBasic.hpp"
#include "services/network/HttpClientImpl.hpp"
#include "services/network/HttpServer.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"
#include "gmock/gmock.h"

struct HttpResponse
{
    services::HttpStatusCode status;
    std::vector<services::HttpHeader> headers;
    std::string body;

    std::function<void()> onDone;
    std::function<void()> onError;
};

class InspectingHttpClient
    : private services::HttpClientBasic
{
public:
    InspectingHttpClient(infra::BoundedString url, uint16_t port, services::HttpClientConnector& connector, HttpResponse& response)
        : HttpClientBasic(url, port, connector)
        , response(response)
    {}

private:
    void Attached() override
    {
        HttpClientObserver::Subject().Get(Path(), Headers());
    }

    void StatusAvailable(services::HttpStatusCode statusCode) override
    {
        response.status = statusCode;
    }

    void HeaderAvailable(services::HttpHeader header) override
    {
        response.headers.emplace_back(header);
    }

    void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) override
    {
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);

        while (!stream.Empty())
            response.body += infra::ByteRangeAsStdString(stream.ContiguousRange());
    }

    void Done() override
    {
        response.onDone();
    }

    void Error(bool intermittentFailure) override
    {
        response.onError();
    }

private:
    HttpResponse& response;
};

struct HttpClientContext
{
    void HttpRequest(const std::string& url)
    {
        requestUrl = url;
        std::atomic<bool> running{ true };

        response.onDone = [&running]
        {
            running = false;
        };

        std::thread requestThread([this, &running]
            {
                httpClient.Emplace(requestUrl, 80, connector, response);

                while (running)
                {}
            });

        requestThread.join();
    }

    services::ConnectionFactoryWithNameResolverImpl::WithStorage<1> connectionFactory{ main_::NetworkAdapter::Instance().ConnectionFactory(), main_::NetworkAdapter::Instance().NameResolver() };
    services::HttpClientConnectorWithNameResolverImpl<> connector{ connectionFactory };
    infra::Optional<InspectingHttpClient> httpClient;
    std::string requestUrl;
    HttpResponse response;
};

GIVEN("I make a HTTP request to '%s'")
{
    services::CucumberScenarioScope<HttpClientContext> context;
    context->HttpRequest(GetStringArgument(arguments, 0)->ToStdString());
}

WHEN("I receive the HTTP response")
{
    services::CucumberScenarioScope<HttpClientContext> context;
}

THEN("the response status code is %d")
{
}

THEN("the response body contains '%s'")
{
    // services::CucumberScenarioScope<HttpClientContext> context;
    // EXPECT_THAT(context->response.body, ::testing::StrEq(GetStringArgument(arguments, 0)->ToStdString()));
    // EXPECT_EQ(context->response.body, GetStringArgument(arguments, 0)->ToStdString());
    EXPECT_THAT(0, ::testing::Eq(1));
}
