#include "services/network/HttpClientBasic.hpp"

namespace services
{
    const NoAutoConnect noAutoConnect;

    HttpClientBasic::HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector)
        : HttpClientBasic(url, port, httpClientConnector, std::chrono::minutes(1))
    {}

    HttpClientBasic::HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector, infra::Duration timeoutDuration)
        : HttpClientBasic(url, port, httpClientConnector, timeoutDuration, noAutoConnect)
    {
        Connect();
    }

    HttpClientBasic::HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector, NoAutoConnect)
        : HttpClientBasic(url, port, httpClientConnector, std::chrono::minutes(1), noAutoConnect)
    {}

    HttpClientBasic::HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector, infra::Duration timeoutDuration, NoAutoConnect)
        : httpClientConnector(httpClientConnector)
        , sharedAccess([this]()
              { Expire(); })
        , url(url)
        , port(port)
        , timeoutDuration(timeoutDuration)
    {}

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
        assert(state == State::idle);
        state = State::connecting;
        httpClientConnector.Connect(*this);
    }

    void HttpClientBasic::Connect(infra::BoundedString url)
    {
        this->url = url;
        Connect();
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

    void HttpClientBasic::Detaching()
    {
        if (state == State::connected)
        {
            state = State::closing;
            sharedAccess.SetAction([this]()
                { ReportError(true); });
        }
    }

    void HttpClientBasic::BodyComplete()
    {
        timeoutTimer.Cancel();
        Close();
    }

    void HttpClientBasic::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        StartTimeout();
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
        StartTimeout();
        Established();
        createdClientObserver(sharedAccess.MakeShared(static_cast<services::HttpClientObserver&>(*this)));
    }

    void HttpClientBasic::ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason reason)
    {
        ReportError(true);
    }

    void HttpClientBasic::Close()
    {
        assert(HttpClientObserver::IsAttached());
        assert(state == State::connected || state == State::closing);

        if (state == State::connected)
        {
            state = State::closing;
            timeoutTimer.Cancel();
            HttpClientObserver::Subject().Close();
        }
    }

    void HttpClientBasic::StartTimeout()
    {
        timeoutTimer.Start(timeoutDuration, [this]()
            { Timeout(); });
    }

    void HttpClientBasic::Timeout()
    {
        sharedAccess.SetAction([this]()
            { ReportError(true); });
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
