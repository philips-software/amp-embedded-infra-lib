#ifndef SERVICES_HTTP_CLIENT_BASIC_HPP
#define SERVICES_HTTP_CLIENT_BASIC_HPP

#include "infra/timer/Timer.hpp"
#include "services/network/HttpClient.hpp"

namespace services
{
    struct NoAutoConnect
    {};

    extern const NoAutoConnect noAutoConnect;

    class HttpClientBasic
        : protected services::HttpClientObserver
        , protected services::HttpClientObserverFactory
    {
    public:
        HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector);
        HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector, infra::Duration timeoutDuration);
        HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector, NoAutoConnect);
        HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector, infra::Duration timeoutDuration, NoAutoConnect);

        void Cancel(const infra::Function<void()>& onDone);

    protected:
        void Connect();
        void Connect(infra::BoundedString url);
        void ContentError();

        infra::BoundedString Url() const;
        virtual infra::BoundedString Path() const;
        virtual services::HttpHeaders Headers() const;

        virtual void Established();
        virtual void Done() = 0;
        virtual void Error(bool intermittentFailure) = 0;

    protected:
        // Implementation of HttpClientObserver
        void Detaching() override;
        void BodyComplete() override;
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    protected:
        // Implementation of HttpClientObserverFactory
        infra::BoundedConstString Hostname() const override;
        uint16_t Port() const override;
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::HttpClientObserver> client)>&& createdClientObserver) override;
        void ConnectionFailed(ConnectFailReason reason) override;

    protected:
        void StartTimeout();

    private:
        void Close();
        void Timeout();
        void Expire();
        void ReportError(bool intermittentFailure);

    private:
        enum class State
        {
            idle,
            connecting,
            connected,
            closing
        };

    private:
        services::HttpClientConnector& httpClientConnector;
        infra::AccessedBySharedPtr sharedAccess;
        infra::BoundedString url;
        uint16_t port;

        infra::Duration timeoutDuration;
        infra::TimerSingleShot timeoutTimer;
        State state = State::idle;
        bool contentError = false;
    };
}

#endif
