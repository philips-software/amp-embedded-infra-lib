#ifndef DI_COMM_HTTP_ERRORS_HPP
#define DI_COMM_HTTP_ERRORS_HPP

#include "services/network/HttpServer.hpp"

namespace services
{
    class HttpErrorResponse
        : public HttpResponse
    {
    public:
        HttpErrorResponse(const char* status, const char* body);

    protected:
        virtual infra::BoundedConstString Status() const override;
        virtual void WriteBody(infra::TextOutputStream& stream) const override;
        virtual infra::BoundedConstString ContentType() const override;

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
