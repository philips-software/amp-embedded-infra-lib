#ifndef SERVICES_NETWORK_HTTP_CLIENT_AUTHENTICATION_HPP
#define SERVICES_NETWORK_HTTP_CLIENT_AUTHENTICATION_HPP

#include "infra/util/BoundedVector.hpp"
#include "services/network/HttpClient.hpp"

namespace services
{
    class HttpClientAuthentication
        : public HttpClient
        , public HttpClientObserver
    {
    public:
        HttpClientAuthentication(infra::BoundedVector<HttpHeader>& headersWithAuthorization);

        // Implementation of HttpClient
        virtual void Get(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Head(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Connect(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Options(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Post(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers = noHeaders) override;
        virtual void Post(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Put(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers = noHeaders) override;
        virtual void Put(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void AckReceived() override;
        virtual void Close() override;
        virtual Connection& GetConnection() override;

        // Implementation of HttpClientObserver
        virtual void StatusAvailable(HttpStatusCode statusCode) override;
        virtual void HeaderAvailable(HttpHeader header) override;
        virtual void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) override;
        virtual void BodyComplete() override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void FillContent(infra::StreamWriter& writer) const override;
        virtual void Detaching() override;

    protected:
        virtual void Authenticate(infra::BoundedConstString scheme, infra::BoundedConstString value) = 0;
        virtual infra::BoundedConstString AuthenticationHeader() const = 0;
        virtual bool Retry() const = 0;

    private:
        void Request(HttpHeaders headers, const infra::Function<void(), sizeof(void*) + 2 * sizeof(infra::BoundedConstString)>& newRequest);
        void MakeHeaders(HttpHeaders headers);
        void Authenticate(infra::BoundedConstString headerValue);

    private:
        infra::Function<void(), sizeof(void*) + 2 * sizeof(infra::BoundedConstString)> request;
        bool unauthorized = false;
        infra::BoundedVector<HttpHeader>& headersWithAuthorization;
    };

    class HttpClientAuthenticationConnector
        : public HttpClientConnector
        , private HttpClientObserverFactory
    {
    public:
        HttpClientAuthenticationConnector(HttpClientConnector& connector, HttpClientAuthentication& clientAuthentication);

        // Implementation of HttpClientConnector
        virtual void Connect(HttpClientObserverFactory& factory) override;
        virtual void CancelConnect(HttpClientObserverFactory& factory) override;

    private:
        // Implementation of HttpClientObserverFactory
        virtual infra::BoundedConstString Hostname() const override;
        virtual uint16_t Port() const override;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)>&& createdClientObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;

    private:
        HttpClientConnector& connector;
        HttpClientAuthentication& clientAuthentication;
        HttpClientObserverFactory* factory = nullptr;
    };
}

#endif
