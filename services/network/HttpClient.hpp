#ifndef SERVICES_NETWORK_HTTP_CLIENT_HPP
#define SERVICES_NETWORK_HTTP_CLIENT_HPP

#include "services/network/Connection.hpp"
#include "services/network/Http.hpp"

namespace services
{
    class HttpClient;

    class HttpClientObserver
        : public infra::SharedOwnedObserver<HttpClientObserver, HttpClient>
    {
    public:
        virtual void StatusAvailable(HttpStatusCode statusCode) = 0;
        virtual void HeaderAvailable(HttpHeader header) = 0;
        virtual void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) = 0;
        virtual void BodyComplete() = 0;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) {}
        virtual void FillContent(infra::StreamWriter& writer) const {}
        virtual void CloseRequested() {} // By default, HTTP Clients finish their request
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
        enum class ConnectFailReason
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
    
    class HttpClient
        : public infra::SharedOwningSubject<HttpClientObserver>
    {
    public:
        virtual void Get(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) = 0;
        virtual void Head(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) = 0;
        virtual void Connect(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) = 0;
        virtual void Options(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) = 0;

        // Full content is given as an argument
        virtual void Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) = 0;
        // With this overload, the size of the content is given, and the observer receives repeated SendStreamAvailable calls until it provides
        // enough content to fill the content size
        virtual void Post(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers = noHeaders) = 0;
        // No content is given, but the observer receives multiple FillContent calls. Each invocation must fill out the exact same contents.
        virtual void Post(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) = 0;

        virtual void Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) = 0;
        virtual void Put(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers = noHeaders) = 0;
        virtual void Put(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) = 0;

        // Full content is given as an argument
        virtual void Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) = 0;
        // No content is given, but the observer receives multiple FillContent calls. Each invocation must fill out the exact same contents.
        virtual void Patch(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) = 0;
        virtual void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) = 0;

        virtual void AckReceived() = 0;
        virtual void CloseConnection() = 0;

        virtual Connection& GetConnection() = 0;
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
}

#endif
