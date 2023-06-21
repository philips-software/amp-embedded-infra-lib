#ifndef SERVICES_NETWORK_HTTP_HPP
#define SERVICES_NETWORK_HTTP_HPP

#include "infra/stream/InputStream.hpp"
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
        PermanentRedirect = 308,
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

    namespace http_responses
    {
        extern const char* ok;                  // 200
        extern const char* badRequest;          // 400
        extern const char* notFound;            // 404
        extern const char* conflict;            // 409
        extern const char* unprocessableEntity; // 422
        extern const char* tooManyRequests;     // 429
        extern const char* internalServerError; // 500
        extern const char* notImplemented;      // 501
    }

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

    extern const struct Chunked
    {
    } chunked;

    class HttpRequestFormatter
    {
    public:
        HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, const HttpHeaders headers);
        HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers);
        HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, std::size_t contentSize, const HttpHeaders headers);
        HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, const HttpHeaders headers, Chunked);

        std::size_t Size() const;
        std::size_t Write(infra::TextOutputStream stream) const;
        void Consume(std::size_t amount);

    private:
        void AddContentLength(std::size_t size);
        std::size_t HeadersSize() const;

    private:
        HttpVerb verb;
        infra::BoundedConstString requestTarget;
        infra::BoundedConstString content;
        infra::BoundedString::WithStorage<8> contentLength;
        infra::Optional<HttpHeader> contentLengthHeader;
        HttpHeader hostHeader;
        const HttpHeaders headers;
        bool chunked{ false };
        bool sentHeader{ false };
    };

    class HttpHeaderParserObserver
    {
    protected:
        HttpHeaderParserObserver() = default;
        HttpHeaderParserObserver(const HttpHeaderParserObserver& other) = delete;
        HttpHeaderParserObserver& operator=(const HttpHeaderParserObserver& other) = delete;
        ~HttpHeaderParserObserver() = default;

    public:
        virtual void StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine) = 0;
        virtual void HeaderAvailable(HttpHeader header) = 0;
        virtual void HeaderParsingDone(bool error) = 0;
    };

    class HttpHeaderParser
    {
    public:
        explicit HttpHeaderParser(HttpHeaderParserObserver& observer);
        ~HttpHeaderParser();

        void DataReceived(infra::StreamReaderWithRewinding& reader);

    private:
        void ParseStatusLine(infra::StreamReaderWithRewinding& reader, bool& error);
        bool HttpVersionValid(infra::BoundedConstString httpVersion);

        void ParseHeaders(infra::StreamReaderWithRewinding& reader);
        HttpHeader HeaderFromString(infra::BoundedConstString header);

    private:
        HttpHeaderParserObserver& observer;
        bool statusParsed = false;
        HttpStatusCode statusCode;
        bool* destroyed = nullptr;
    };

    infra::BoundedConstString SchemeFromUrl(infra::BoundedConstString url);
    infra::BoundedConstString HostAndPortFromUrl(infra::BoundedConstString url);
    infra::BoundedConstString HostFromUrl(infra::BoundedConstString url);
    infra::Optional<uint16_t> PortFromUrl(infra::BoundedConstString url);
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
