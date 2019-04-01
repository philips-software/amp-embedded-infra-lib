#ifndef SERVICES_NETWORK_HTTP_HPP
#define SERVICES_NETWORK_HTTP_HPP

#include "services/network/Connection.hpp"
#include "infra/util/BoundedVector.hpp"

namespace services
{
    class HttpHeader;
    class HttpClient;

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

    class HttpClientObserver
        : public infra::SingleObserver<HttpClientObserver, HttpClient>
    {
    public:
        virtual void Connected() {};
        virtual void ClosingConnection() {};

        virtual void StatusAvailable(HttpStatusCode statusCode) = 0;
        virtual void HeaderAvailable(HttpHeader header) = 0;
        virtual void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) = 0;
        virtual void BodyComplete() = 0;
    };

    class HttpClientObserverFactory
        : public infra::IntrusiveList<HttpClientObserverFactory>::NodeType
    {
    protected:
        HttpClientObserverFactory() = default;
        HttpClientObserverFactory(const HttpClientObserverFactory& other) = delete;
        HttpClientObserverFactory& operator=(const HttpClientObserverFactory& other) = delete;
        ~HttpClientObserverFactory() = default;

    public:
        enum ConnectFailReason
        {
            refused,
            connectionAllocationFailed,
            nameLookupFailed
        };

        virtual infra::BoundedConstString Hostname() const = 0;
        virtual uint16_t Port() const = 0;

        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)>&& createdClientObserver) = 0;
        virtual void ConnectionFailed(ConnectFailReason reason) = 0;
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
    static const HttpHeaders noHeaders{};

    class HttpClient
        : public infra::Subject<HttpClientObserver>
    {
    public:
        virtual void Get(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) = 0;
        virtual void Head(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) = 0;
        virtual void Connect(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) = 0;
        virtual void Options(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) = 0;
        virtual void Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) = 0;
        virtual void Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) = 0;
        virtual void Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) = 0;
        virtual void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) = 0;

        virtual void AckReceived() = 0;
        virtual void Close() = 0;
    };

    class HttpClientConnector
    {
    protected:
        HttpClientConnector() = default;
        HttpClientConnector(const HttpClientConnector& other) = delete;
        HttpClientConnector& operator=(const HttpClientConnector& other) = delete;
        ~HttpClientConnector() = default;

    public:
        virtual void Connect(HttpClientObserverFactory& factory) = 0;
        virtual void CancelConnect(HttpClientObserverFactory& factory) = 0;
    };

	infra::BoundedConstString SchemeFromUrl(infra::BoundedConstString url);
    infra::BoundedConstString HostFromUrl(infra::BoundedConstString url);
    infra::BoundedConstString PathFromUrl(infra::BoundedConstString url);
}

namespace infra
{
    TextOutputStream& operator<<(TextOutputStream& stream, const services::HttpHeader& header);
	TextOutputStream& operator<<(TextOutputStream& stream, services::HttpStatusCode statusCode);
}

#endif
