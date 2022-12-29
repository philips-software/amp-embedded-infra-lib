#include "infra/event/EventDispatcher.hpp"
#include "services/network/HttpClientCachedConnection.hpp"

namespace services
{
    HttpClientCachedConnection::HttpClientCachedConnection(HttpClientCachedConnectionConnector& connector)
        : connector(connector)
    {}

    bool HttpClientCachedConnection::Idle() const
    {
        return idle;
    }

    void HttpClientCachedConnection::Get(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        idle = false;
        Subject().Get(requestTarget, headers);
    }

    void HttpClientCachedConnection::Head(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        idle = false;
        Subject().Head(requestTarget, headers);
    }

    void HttpClientCachedConnection::Connect(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        idle = false;
        Subject().Connect(requestTarget, headers);
    }

    void HttpClientCachedConnection::Options(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        idle = false;
        Subject().Options(requestTarget, headers);
    }

    void HttpClientCachedConnection::Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        idle = false;
        Subject().Post(requestTarget, content, headers);
    }

    void HttpClientCachedConnection::Post(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers)
    {
        idle = false;
        Subject().Post(requestTarget, contentSize, headers);
    }

    void HttpClientCachedConnection::Post(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        idle = false;
        Subject().Post(requestTarget, headers);
    }

    void HttpClientCachedConnection::Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        idle = false;
        Subject().Put(requestTarget, content, headers);
    }

    void HttpClientCachedConnection::Put(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers)
    {
        idle = false;
        Subject().Put(requestTarget, contentSize, headers);
    }

    void HttpClientCachedConnection::Put(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        idle = false;
        Subject().Put(requestTarget, headers);
    }

    void HttpClientCachedConnection::Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        idle = false;
        Subject().Patch(requestTarget, content, headers);
    }

    void HttpClientCachedConnection::Patch(infra::BoundedConstString requestTarget, HttpHeaders headers)
    {
        idle = false;
        Subject().Patch(requestTarget, headers);
    }

    void HttpClientCachedConnection::Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers)
    {
        idle = false;
        Subject().Delete(requestTarget, content, headers);
    }

    void HttpClientCachedConnection::AckReceived()
    {
        Subject().AckReceived();
    }

    void HttpClientCachedConnection::Close()
    {
        if (HttpClient::IsAttached())
            HttpClient::Detach();
    }

    Connection& HttpClientCachedConnection::GetConnection()
    {
        return Subject().GetConnection();
    }

    void HttpClientCachedConnection::Detaching()
    {
        if (HttpClient::IsAttached())
            HttpClient::Detach();
    }

    void HttpClientCachedConnection::DetachingObserver()
    {
        connector.DetachingObserver();
    }

    void HttpClientCachedConnection::StatusAvailable(HttpStatusCode statusCode)
    {
        Observer().StatusAvailable(statusCode);
    }

    void HttpClientCachedConnection::HeaderAvailable(HttpHeader header)
    {
        Observer().HeaderAvailable(header);
    }

    void HttpClientCachedConnection::BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader)
    {
        Observer().BodyAvailable(std::move(reader));
    }

    void HttpClientCachedConnection::BodyComplete()
    {
        idle = true;
        Observer().BodyComplete();
    }

    void HttpClientCachedConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        Observer().SendStreamAvailable(std::move(writer));
    }

    void HttpClientCachedConnection::FillContent(infra::StreamWriter& writer) const
    {
        Observer().FillContent(writer);
    }

    HttpClientCachedConnectionConnector::HttpClientCachedConnectionConnector(HttpClientConnector& delegate, const Sha256& hasher, infra::Duration disconnectTimeout)
        : delegate(delegate)
        , hasher(hasher)
        , disconnectTimeout(disconnectTimeout)
    {}

    void HttpClientCachedConnectionConnector::Connect(HttpClientObserverFactory& factory)
    {
        waitingClientObserverFactories.push_back(factory);

        infra::EventDispatcher::Instance().Schedule([this]() { TryConnectWaiting(); });
    }

    void HttpClientCachedConnectionConnector::CancelConnect(HttpClientObserverFactory& factory)
    {
        if (clientObserverFactory == &factory)
        {
            delegate.CancelConnect(*this);
            clientObserverFactory = nullptr;
        }
        else
            waitingClientObserverFactories.erase(factory);

        TryConnectWaiting();
    }

    infra::BoundedConstString HttpClientCachedConnectionConnector::Hostname() const
    {
        return clientObserverFactory->Hostname();
    }

    uint16_t HttpClientCachedConnectionConnector::Port() const
    {
        return clientObserverFactory->Port();
    }

    void HttpClientCachedConnectionConnector::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)>&& createdObserver)
    {
        assert(clientObserverFactory != nullptr);
        client.Emplace(*this);
        auto httpClientPtr = clientPtr.MakeShared(*client);

        clientObserverFactory->ConnectionEstablished([&httpClientPtr, &createdObserver](infra::SharedPtr<HttpClientObserver> observer)
            {
                if (observer != nullptr)
                {
                    createdObserver(httpClientPtr);
                    httpClientPtr->Attach(observer);
                }
            });

        clientObserverFactory = nullptr;
    }

    void HttpClientCachedConnectionConnector::ConnectionFailed(ConnectFailReason reason)
    {
        assert(clientObserverFactory != nullptr);
        clientObserverFactory->ConnectionFailed(reason);
        clientObserverFactory = nullptr;
        TryConnectWaiting();
    }

    void HttpClientCachedConnectionConnector::RetargetConnection()
    {
        auto httpClientPtr = clientPtr.MakeShared(*client);

        clientObserverFactory->ConnectionEstablished([httpClientPtr](infra::SharedPtr<HttpClientObserver> observer)
            {
                if (observer)
                    httpClientPtr->Attach(observer);
            });

        clientObserverFactory = nullptr;
    }

    void HttpClientCachedConnectionConnector::Connect()
    {
        if (client != infra::none && client->HttpClientObserver::IsAttached())
            client->HttpClientObserver::Subject().Close();

        hostAndPortHash = GenerateHostAndPortHash(*clientObserverFactory);
        delegate.Connect(*this);
    }

    void HttpClientCachedConnectionConnector::DetachingObserver()
    {
        if (clientObserverFactory == nullptr && client != infra::none && !disconnectTimer.Armed())
            disconnectTimer.Start(disconnectTimeout, [this]() { DisconnectTimeout(); });

        infra::EventDispatcher::Instance().Schedule([this]() { TryConnectWaiting(); });
    }

    void HttpClientCachedConnectionConnector::DisconnectTimeout()
    {
        if (client != infra::none && !client->HttpClient::IsAttached() && client->HttpClientObserver::IsAttached())
            client->HttpClientObserver::Subject().Close();
    };

    void HttpClientCachedConnectionConnector::TryConnectWaiting()
    {
        if (clientObserverFactory == nullptr && (client == infra::none || !client->HttpClient::IsAttached()) && !waitingClientObserverFactories.empty())
        {
            disconnectTimer.Cancel();

            clientObserverFactory = &waitingClientObserverFactories.front();
            waitingClientObserverFactories.pop_front();

            if (client == infra::none || !client->HttpClientObserver::IsAttached() || !client->Idle())
                Connect();
            else
                TryRetargetConnection();
        }
    }

    void HttpClientCachedConnectionConnector::TryRetargetConnection()
    {
        if (SameHost())
            RetargetConnection();
        else
            Connect();
    }

    void HttpClientCachedConnectionConnector::ClientPtrExpired()
    {
        if (client != infra::none && client->HttpClient::IsAttached())
            client->HttpClient::Detach();

        TryConnectWaiting();
    }

    bool HttpClientCachedConnectionConnector::SameHost() const
    {
        return GenerateHostAndPortHash(*clientObserverFactory) == hostAndPortHash;
    }

    std::array<uint8_t, 32> HttpClientCachedConnectionConnector::GenerateHostAndPortHash(const HttpClientObserverFactory& factory) const
    {
        std::array<uint8_t, 32> hostHash = hasher.Calculate(infra::StringAsByteRange(factory.Hostname()));
        auto port = factory.Port();
        std::array<uint8_t, 32> portHash = hasher.Calculate(infra::MakeByteRange(port));

        std::array<uint8_t, 32> result;

        for (int i = 0; i != result.size(); ++i)
            result[i] = hostHash[i] ^ portHash[i];

        return result;
    }
}
