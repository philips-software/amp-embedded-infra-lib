#ifndef EXAMPLES_TIME_HTTP_PAGE_HPP
#define EXAMPLES_TIME_HTTP_PAGE_HPP

#include "services/network/HttpServer.hpp"

namespace application
{
    class TimeHttpPage
        : public services::HttpPage
    {
    public:
        // Implementation of services::HttpPage
        virtual bool ServesRequest(const infra::Tokenizer& pathTokens) const override;
        virtual void RespondToRequest(services::HttpRequestParser& parser, services::HttpServerConnection& connection) override;

    private:
        class TimeResponse
            : public services::HttpResponse
        {
        public:
            TimeResponse();

            // Implementation of services::HttpResponse
            virtual infra::BoundedConstString Status() const override;
            virtual infra::BoundedConstString ContentType() const override;
            virtual void WriteBody(infra::TextOutputStream& stream) const override;
            virtual void AddHeaders(services::HttpResponseHeaderBuilder& builder) const override;
        };

        TimeResponse timeResponse;
    };
}

#endif
