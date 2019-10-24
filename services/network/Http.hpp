#ifndef SERVICES_NETWORK_HTTP_HPP
#define SERVICES_NETWORK_HTTP_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/BoundedString.hpp"

namespace services
{
    class HttpHeader;

    enum class HttpVerb : uint8_t
    {
        get,
        head,
        connect,
        options,
        patch,
        put,
        post,
        delete_
    };

    enum class HttpStatusCode : uint16_t
    {
        Continue = 100,
        SwitchingProtocols = 101,
        OK = 200,
        Created = 201,
        Accepted = 202,
        NonAuthorativeInformation = 203,
        NoContent = 204,
        ResetContent = 205,
        PartialContent = 206,
        MultipleChoices = 300,
        MovedPermanently = 301,
        Found = 302,
        SeeOther = 303,
        NotModified = 304,
        UseProxy = 305,
        TemporaryRedirect = 307,
        BadRequest = 400,
        Unauthorized = 401,
        PaymentRequired = 402,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        NotAcceptable = 406,
        ProxyAuthenticationRequired = 407,
        RequestTimeOut = 408,
        Conflict = 409,
        Gone = 410,
        LengthRequired = 411,
        PreconditionFailed = 412,
        RequestEntityTooLarge = 413,
        RequestUriTooLarge = 414,
        UnsupportedMediaType = 415,
        RequestRangeNotSatisfiable = 416,
        ExpectationFailed = 417,
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503,
        GatewayTimeOut = 504,
        HttpVersionNotSupported = 505
    };

    class HttpHeader
    {
    public:
        HttpHeader(infra::BoundedConstString field, infra::BoundedConstString value);

        std::size_t Size() const;
        infra::BoundedConstString Field() const;
        infra::BoundedConstString Value() const;

        bool operator==(const HttpHeader& rhs) const;

    private:
        infra::BoundedConstString field;
        infra::BoundedConstString value;
    };

    using HttpHeaders = infra::MemoryRange<const HttpHeader>;
    extern const HttpHeaders noHeaders;

    infra::BoundedConstString SchemeFromUrl(infra::BoundedConstString url);
    infra::BoundedConstString HostFromUrl(infra::BoundedConstString url);
    infra::BoundedConstString PathFromUrl(infra::BoundedConstString url);

    infra::BoundedConstString HttpVerbToString(HttpVerb verb);
    infra::Optional<HttpVerb> HttpVerbFromString(infra::BoundedConstString verb);

    infra::Optional<HttpStatusCode> HttpStatusCodeFromString(infra::BoundedConstString statusCode);
    infra::BoundedConstString HttpStatusCodeToString(services::HttpStatusCode statusCode);
}

namespace infra
{
    TextOutputStream& operator<<(TextOutputStream& stream, const services::HttpHeader& header);
    TextOutputStream& operator<<(TextOutputStream& stream, services::HttpVerb verb);
    TextOutputStream& operator<<(TextOutputStream& stream, services::HttpStatusCode statusCode);
}

#endif
