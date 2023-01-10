#ifndef SERVICES_HTTP_CLIENT_CACHED_CONNECTION_HPP
#define SERVICES_HTTP_CLIENT_CACHED_CONNECTION_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/SharedPtr.hpp"
#include "services/network/HttpClient.hpp"
#include "services/util/Sha256.hpp"

namespace services
{
    class HttpClientCachedConnectionConnector;

    class HttpClientCachedConnection
        : public HttpClient
        , public HttpClientObserver
    {
    public:
        HttpClientCachedConnection(HttpClientCachedConnectionConnector& connector, infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)>&& createdObserver);

        bool Idle() const;

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
        virtual void Patch(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void AckReceived() override;
        virtual void Close() override;
        virtual Connection& GetConnection() override;

        // Implementation of SharedOwnedObserver via HttpClientObserver
        virtual void Detaching() override;

        // Implementation of SharedOwnedSubject via HttpClient
        virtual void DetachingObserver() override;

    public:
        // Implementation of HttpClientObserver
        virtual void StatusAvailable(HttpStatusCode statusCode) override;
        virtual void HeaderAvailable(HttpHeader header) override;
        virtual void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) override;
        virtual void BodyComplete() override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void FillContent(infra::StreamWriter& writer) const override;

    private:
        friend class HttpClientCachedConnectionConnector;

        HttpClientCachedConnectionConnector& connector;
        infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)> createdObserver;
        bool idle = true;
    };

    class HttpClientCachedConnectionConnector
        : public HttpClientConnector
        , private HttpClientObserverFactory
    {
    public:
        HttpClientCachedConnectionConnector(HttpClientConnector& delegate, const Sha256& hasher, infra::Duration disconnectTimeout = std::chrono::minutes(1));

        // Implementation of HttpClientConnector
        virtual void Connect(HttpClientObserverFactory& factory) override;
        virtual void CancelConnect(HttpClientObserverFactory& factory) override;

    private:
        // Implementation of HttpClientObserverFactory
        virtual infra::BoundedConstString Hostname() const override;
        virtual uint16_t Port() const override;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)>&& createdObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;

    protected:
        friend class HttpClientCachedConnection;
        
        virtual void RetargetConnection();
        virtual void Connect();
        virtual void DetachingObserver();
        virtual void DisconnectTimeout();

    private:
        void TryConnectWaiting();
        void TryRetargetConnection();
        void ClientPtrExpired();
        bool SameHost() const;
        Sha256::Digest GenerateHostAndPortHash(const HttpClientObserverFactory& factory) const;

    private:
        HttpClientConnector& delegate;
        const Sha256& hasher;

        HttpClientObserverFactory* clientObserverFactory = nullptr;
        infra::IntrusiveList<HttpClientObserverFactory> waitingClientObserverFactories;

        infra::Optional<HttpClientCachedConnection> client;
        infra::AccessedBySharedPtr clientPtr{ [this]() { ClientPtrExpired(); } };

        Sha256::Digest hostAndPortHash{};

        infra::Duration disconnectTimeout;
        infra::TimerSingleShot disconnectTimer;
    };
}

#endif
