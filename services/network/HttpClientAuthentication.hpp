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
        explicit HttpClientAuthentication(infra::BoundedVector<HttpHeader>& headersWithAuthorization);

        // Implementation of HttpClient
        void Get(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Head(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Connect(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Options(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        void Post(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        void Put(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        void Patch(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        void AckReceived() override;
        void CloseConnection() override;
        Connection& GetConnection() override;

        // Implementation of HttpClientObserver
        void StatusAvailable(HttpStatusCode statusCode) override;
        void HeaderAvailable(HttpHeader header) override;
        void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) override;
        void BodyComplete() override;
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void FillContent(infra::StreamWriter& writer) const override;
        void Detaching() override;

    protected:
        virtual void Authenticate(infra::BoundedConstString scheme, infra::BoundedConstString value) = 0;
        virtual infra::BoundedConstString AuthenticationHeader() const = 0;
        virtual bool Retry() const = 0;
        virtual void Reset() = 0;

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
        void Connect(HttpClientObserverFactory& factory) override;
        void CancelConnect(HttpClientObserverFactory& factory) override;

    private:
        // Implementation of HttpClientObserverFactory
        infra::BoundedConstString Hostname() const override;
        uint16_t Port() const override;
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)>&& createdClientObserver) override;
        void ConnectionFailed(ConnectFailReason reason) override;

    private:
        HttpClientConnector& connector;
        HttpClientAuthentication& clientAuthentication;
        HttpClientObserverFactory* factory = nullptr;
    };
}

#endif
