#include "infra/event/EventDispatcher.hpp"
#include "services/network/HttpClientCachedConnection.hpp"

namespace services
{
    HttpClientCachedConnection::HttpClientCachedConnection(HttpClientCachedConnectionConnector& connector, infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)>&& createdObserver)
        : connector(connector)
        , createdObserver(std::move(createdObserver))
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

    void HttpClientCachedConnection::CloseConnection()
    {
        if (HttpClient::IsAttached() && !detachingObserver)
            HttpClient::Detach();
    }

    Connection& HttpClientCachedConnection::GetConnection()
    {
        return Subject().GetConnection();
    }

    void HttpClientCachedConnection::CloseRequested()
    {
        if (HttpClient::IsAttached() && !detachingObserver)
        {
            closeRequested = true;
            Observer().CloseRequested();
        }
        else
            HttpClientObserver::Subject().CloseConnection();
    }

    void HttpClientCachedConnection::Detaching()
    {
        detaching = true;

        if (HttpClient::IsAttached() && !detachingObserver)
            HttpClient::Detach();
    }

    void HttpClientCachedConnection::AttachedObserver()
    {
        detachingObserver = false;
    }

    void HttpClientCachedConnection::DetachingObserver()
    {
        detachingObserver = true;

        if (!detaching && closeRequested)
            HttpClientObserver::Subject().CloseConnection();
        else
            connector.DetachingObserver();
    }

    void HttpClientCachedConnection::StatusAvailable(HttpStatusCode statusCode)
    {
        Observer().StatusAvailable(statusCode);
    }

    void HttpClientCachedConnection::HeaderAvailable(HttpHeader header)
    {
        if (HttpClient::IsAttached())
            Observer().HeaderAvailable(header);
    }

    void HttpClientCachedConnection::BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader)
    {
        if (HttpClient::IsAttached())
            Observer().BodyAvailable(std::move(reader));
    }

    void HttpClientCachedConnection::BodyComplete()
    {
        idle = true;
        if (HttpClient::IsAttached())
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

        infra::EventDispatcher::Instance().Schedule([this]() { TryConnectWaiting(); });
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
        // createdObserver is stored temporarily (until it is invoked) inside the client, so that if the lambda passed to ConnectionEstablished
        // is discarded instead of used, then createdObserver is discarded automatically as well.
        client.Emplace(*this, std::move(createdObserver));

        clientObserverFactory->ConnectionEstablished([httpClientPtr = clientPtr.MakeShared(*client)](infra::SharedPtr<HttpClientObserver> observer)
            {
                if (observer != nullptr)
                {
                    httpClientPtr->createdObserver(httpClientPtr);
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
        clientObserverFactory->ConnectionEstablished([httpClientPtr = clientPtr.MakeShared(*client)](infra::SharedPtr<HttpClientObserver> observer)
            {
                if (observer != nullptr)
                    httpClientPtr->Attach(observer);
            });

        clientObserverFactory = nullptr;
    }

    void HttpClientCachedConnectionConnector::Connect()
    {
        if (client != infra::none && client->HttpClientObserver::IsAttached())
            client->HttpClientObserver::Subject().CloseConnection();

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
            client->HttpClientObserver::Subject().CloseConnection();
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
        if (client != infra::none && client->HttpClient::IsAttached() && !client->detachingObserver)
            client->HttpClient::Detach();

        TryConnectWaiting();
    }

    bool HttpClientCachedConnectionConnector::SameHost() const
    {
        return GenerateHostAndPortHash(*clientObserverFactory) == hostAndPortHash;
    }

    Sha256::Digest HttpClientCachedConnectionConnector::GenerateHostAndPortHash(const HttpClientObserverFactory& factory) const
    {
        Sha256::Digest hostHash = hasher.Calculate(infra::StringAsByteRange(factory.Hostname()));
        auto port = factory.Port();
        Sha256::Digest portHash = hasher.Calculate(infra::MakeByteRange(port));

        Sha256::Digest result;

        for (int i = 0; i != result.size(); ++i)
            result[i] = hostHash[i] ^ portHash[i];

        return result;
    }
}
