#ifndef SERVICES_HTTP_ERRORS_HPP
#define SERVICES_HTTP_ERRORS_HPP

#include "services/network/HttpServer.hpp"

namespace services
{
    class HttpErrorResponse
        : public HttpResponse
    {
    public:
        HttpErrorResponse(const char* status, const char* body);

    protected:
        infra::BoundedConstString Status() const override;
        void WriteBody(infra::TextOutputStream& stream) const override;
        infra::BoundedConstString ContentType() const override;

    private:
        const char* status;
        const char* body;
    };

    class HttpResponseBadRequest
        : public HttpErrorResponse
    {
    public:
        HttpResponseBadRequest();

        static const HttpResponseBadRequest& Instance();
    };

    class HttpResponseNotFound
        : public HttpErrorResponse
    {
    public:
        HttpResponseNotFound();

        static const HttpResponseNotFound& Instance();
    };

    class HttpResponseMethodNotAllowed
        : public HttpErrorResponse
    {
    public:
        HttpResponseMethodNotAllowed();

        static const HttpResponseMethodNotAllowed& Instance();
    };

    class HttpResponseOutOfMemory
        : public HttpErrorResponse
    {
    public:
        HttpResponseOutOfMemory();

        static const HttpResponseOutOfMemory& Instance();
    };
}

#endif
