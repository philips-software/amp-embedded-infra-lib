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

    void HttpClientBasic::Cancel(const infra::Function<void()>& onDone)
    {
        if (state == State::connecting)
        {
            httpClientConnector.CancelConnect(*this);
            onDone();
        }
        else if (state == State::connected)
        {
            sharedAccess.SetAction(onDone);
            Close();
        }
        else if (state == State::closing)
            sharedAccess.SetAction(onDone);
        else
            onDone();
    }

    void HttpClientBasic::Connect()
    {
        state = State::connecting;
        httpClientConnector.Connect(*this);
    }

    void HttpClientBasic::Close()
    {
        assert(HttpClientObserver::Attached());
        assert(state == State::connected);

        state = State::closing;
        timeoutTimer.Cancel();
        HttpClientObserver::Subject().Close();
    }

    void HttpClientBasic::ContentError()
    {
        contentError = true;
        Close();
    }

    infra::BoundedString HttpClientBasic::Url() const
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

    void HttpClientBasic::ClosingConnection()
    {
        if (state == State::connected)
        {
            state = State::closing;
            sharedAccess.SetAction([this]() { ReportError(true); });
        }

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
        ReportError(true);
    }

    void HttpClientBasic::Timeout()
    {
        sharedAccess.SetAction([this]() { ReportError(true); });
        Close();
    }

    void HttpClientBasic::Expire()
    {
        if (contentError)
            ReportError(false);
        else
        {
            state = State::idle;
            Done();
        }
    }

    void HttpClientBasic::ReportError(bool intermittentFailure)
    {
        state = State::idle;
        timeoutTimer.Cancel();
        Error(intermittentFailure);
    }
}
