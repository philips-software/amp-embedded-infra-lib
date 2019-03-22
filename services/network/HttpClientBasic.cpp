#include "services/network/HttpClientBasic.hpp"

namespace services
{
    HttpClientBasic::HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector)
        : HttpClientBasic(url, port, httpClientConnector, std::chrono::minutes(1))
    {}

    HttpClientBasic::HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector, infra::Duration timeoutDuration)
        : httpClientConnector(httpClientConnector)
        , sharedAccess([this]() { Expire(); })
        , url(url)
        , port(port)
        , timeoutDuration(timeoutDuration)
    {
        Connect();
    }

    void HttpClientBasic::Connect()
    {
        state = State::connecting;
        httpClientConnector.Connect(*this);
    }

    void HttpClientBasic::Cancel(const infra::Function<void()>& onDone)
    {
        if (state == State::connecting)
            httpClientConnector.CancelConnect(*this);
        else if (state == State::connected)
            Close();

        if (state == State::closing)
            sharedAccess.SetAction(onDone);
        else
            onDone();
    }

    const infra::BoundedString& HttpClientBasic::Url() const
    {
        return url;
    }

    infra::BoundedString HttpClientBasic::Path() const
    {
        auto path = services::PathFromUrl(url);
        auto offset = path.empty() ? url.size() : path.begin() - url.begin();
        return infra::BoundedString(infra::MakeRange(url.begin() + offset, url.begin() + url.max_size() - offset), path.size());
    }

    services::HttpHeaders HttpClientBasic::Headers() const
    {
        return services::noHeaders;
    }

    void HttpClientBasic::Established()
    {}

    void HttpClientBasic::Failed()
    {}

    void HttpClientBasic::TimedOut()
    {}

    void HttpClientBasic::Expired()
    {}

    void HttpClientBasic::ClosingConnection()
    {
        Detach();
    }

    void HttpClientBasic::BodyComplete()
    {
        timeoutTimer.Cancel();
        Close();
    }

    infra::BoundedConstString HttpClientBasic::Hostname() const
    {
        return services::HostFromUrl(url);
    }

    uint16_t HttpClientBasic::Port() const
    {
        return port;
    }

    void HttpClientBasic::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::HttpClientObserver> client)>&& createdClientObserver)
    {
        state = State::connected;
        timeoutTimer.Start(timeoutDuration, [this]() { Timeout(); });
        Established();
        createdClientObserver(sharedAccess.MakeShared(static_cast<services::HttpClientObserver&>(*this)));
    }

    void HttpClientBasic::ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason reason)
    {
        state = State::idle;
        Failed();
    }

    void HttpClientBasic::Close()
    {
        state = State::closing;
        timeoutTimer.Cancel();
        HttpClientObserver::Subject().Close();
    }

    void HttpClientBasic::Timeout()
    {
        sharedAccess.SetAction([this]() { TimedOut(); });
        Close();
    }

    void HttpClientBasic::Expire()
    {
        state = State::idle;
        Expired();
    }
}
