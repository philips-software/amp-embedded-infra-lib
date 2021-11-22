#include "examples/http_server/TimeHttpPage.hpp"
#include "infra/syntax/JsonFormatter.hpp"

namespace application
{
    bool TimeHttpPage::ServesRequest(const infra::Tokenizer& pathTokens) const
    {
        return pathTokens.TokenAndRest(0) == "";
    }

    void TimeHttpPage::RespondToRequest(services::HttpRequestParser& parser, services::HttpServerConnection& connection)
    {
        connection.SendResponse(timeResponse);
    }

    TimeHttpPage::TimeResponse::TimeResponse()
        : services::HttpResponse(128)
    {}

    infra::BoundedConstString TimeHttpPage::TimeResponse::Status() const
    {
        return services::http_responses::ok;
    }

    infra::BoundedConstString TimeHttpPage::TimeResponse::ContentType() const
    {
        return "application/json";
    }

    void TimeHttpPage::TimeResponse::WriteBody(infra::TextOutputStream& stream) const
    {
        infra::JsonObjectFormatter json(stream);
        json.Add("time", std::chrono::system_clock::now().time_since_epoch().count());
    }

    void TimeHttpPage::TimeResponse::AddHeaders(services::HttpResponseHeaderBuilder& builder) const
    {}
}
