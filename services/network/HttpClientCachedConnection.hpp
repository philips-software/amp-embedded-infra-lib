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

        // Implementation of SharedOwnedObserver via HttpClientObserver
        void CloseRequested() override;
        void Detaching() override;

        // Implementation of SharedOwnedSubject via HttpClient
        void AttachedObserver() override;
        void DetachingObserver() override;

    public:
        // Implementation of HttpClientObserver
        void StatusAvailable(HttpStatusCode statusCode) override;
        void HeaderAvailable(HttpHeader header) override;
        void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) override;
        void BodyComplete() override;
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void FillContent(infra::StreamWriter& writer) const override;

    private:
        friend class HttpClientCachedConnectionConnector;

        HttpClientCachedConnectionConnector& connector;
        infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)> createdObserver;
        bool idle = true;
        bool closeRequested = false;
        bool detaching = false;
        bool detachingObserver = false;
    };

    class HttpClientCachedConnectionConnector
        : public HttpClientConnector
        , private HttpClientObserverFactory
    {
    public:
        HttpClientCachedConnectionConnector(HttpClientConnector& delegate, const Sha256& hasher, infra::Duration disconnectTimeout = std::chrono::minutes(1));

        // Implementation of HttpClientConnector
        void Connect(HttpClientObserverFactory& factory) override;
        void CancelConnect(HttpClientObserverFactory& factory) override;

        void Stop(const infra::Function<void()>& onDone);

    private:
        // Implementation of HttpClientObserverFactory
        infra::BoundedConstString Hostname() const override;
        uint16_t Port() const override;
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)>&& createdObserver) override;
        void ConnectionFailed(ConnectFailReason reason) override;

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

        std::optional<HttpClientCachedConnection> client;
        infra::AccessedBySharedPtr clientPtr{ [this]()
            {
                ClientPtrExpired();
            } };

        Sha256::Digest hostAndPortHash{};

        infra::Duration disconnectTimeout;
        infra::TimerSingleShot disconnectTimer;
    };
}

#endif
