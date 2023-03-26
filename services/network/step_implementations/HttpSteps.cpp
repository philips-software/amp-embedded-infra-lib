#include "infra/util/BoundedString.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Optional.hpp"
#include "services/cucumber/CucumberStepMacro.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/HttpClientBasic.hpp"
#include "services/network/HttpClientImpl.hpp"
#include "services/network/HttpServer.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"
#include <vector>

struct HttpResponse
{
    services::HttpStatusCode status;
    std::vector<services::HttpHeader> headers;
    std::string body;
    bool complete = false;
    bool error = false;
};

class InspectingHttpClient
    : private services::HttpClientBasic
{
public:
    InspectingHttpClient(infra::BoundedString url, uint16_t port, services::HttpClientConnector& connector, HttpResponse& response)
        : HttpClientBasic(url, port, connector)
        , response(response)
    {
    }

private:
    virtual void Attached() override
    {
        HttpClientObserver::Subject().Get(Path(), Headers());
    }

    virtual void StatusAvailable(services::HttpStatusCode statusCode) override
    {
        response.status = statusCode;
    }

    virtual void HeaderAvailable(services::HttpHeader header) override
    {
        response.headers.emplace_back(header);
    }

    virtual void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) override
    {
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);

        while (!stream.Empty())
            response.body += infra::ByteRangeAsStdString(stream.ContiguousRange());
    }

    virtual void Done() override
    {
        response.complete = true;
    }

    virtual void Error(bool intermittentFailure) override
    {
        response.error = true;
    }

private:
    HttpResponse& response;
};

struct HttpStepImplementations
{
    infra::Optional<services::DefaultHttpServer::WithBuffer<2048>> httpServer;
    std::vector<services::HttpPageWithContent> pages;

    services::ConnectionFactoryWithNameResolverImpl::WithStorage<1> connectionFactory{ main_::NetworkAdapter::Instance().ConnectionFactory(), main_::NetworkAdapter::Instance().NameResolver() };
    services::HttpClientConnectorWithNameResolverImpl<> connector{ connectionFactory };
    infra::Optional<InspectingHttpClient> httpClient;
    HttpResponse response;
};

static HttpStepImplementations steps;

GIVEN("I start a http server")
{
    steps.httpServer.Emplace(main_::NetworkAdapter::Instance().ConnectionFactory(), 80);
    Success();
}

GIVEN("I add a page with static content")
{
    //auto content = GetStringArgument(arguments, 0)->ToStdString();
    static services::HttpPageWithContent page("", "<html></html>", "text/html");
    Success();
}

WHEN("I make a http request to localhost")
{
    infra::BoundedString::WithStorage<128> url("localhost");
    steps.httpClient.Emplace(url, 80, steps.connector, steps.response);
    Success();
}

THEN("I check that the response is correct")
{
    if (steps.response.body == "<html></html>")
        Success();
    else
        Error("Content does not match");
}
