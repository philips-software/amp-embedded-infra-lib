#include "services/network/HttpErrors.hpp"

namespace services
{
    HttpErrorResponse::HttpErrorResponse(const char* status, const char* body)
        : status(status)
        , body(body)
    {}

    infra::BoundedConstString HttpErrorResponse::Status() const
    {
        return status;
    }

    void HttpErrorResponse::WriteBody(infra::TextOutputStream& stream) const
    {
        stream << body;
    }

    infra::BoundedConstString HttpErrorResponse::ContentType() const
    {
        return "application/json";
    }

    HttpResponseBadRequest::HttpResponseBadRequest()
        : HttpErrorResponse("400 Bad request", "{}")
    {}

    const HttpResponseBadRequest& HttpResponseBadRequest::Instance()
    {
        static const HttpResponseBadRequest instance;

        return instance;
    }

    HttpResponseNotFound::HttpResponseNotFound()
        : HttpErrorResponse("404 Not Found", "{}")
    {}

    const HttpResponseNotFound& HttpResponseNotFound::Instance()
    {
        static const HttpResponseNotFound instance;

        return instance;
    }

    HttpResponseMethodNotAllowed::HttpResponseMethodNotAllowed()
        : HttpErrorResponse("405 Method not allowed", "{}")
    {}

    const HttpResponseMethodNotAllowed& HttpResponseMethodNotAllowed::Instance()
    {
        static const HttpResponseMethodNotAllowed instance;

        return instance;
    }

    HttpResponseOutOfMemory::HttpResponseOutOfMemory()
        : HttpErrorResponse("500 Internal Server Error", R"({ "error": "Out of memory" })")
    {}

    const HttpResponseOutOfMemory& HttpResponseOutOfMemory::Instance()
    {
        static const HttpResponseOutOfMemory instance;

        return instance;
    }
}
