#ifndef SERVICES_HTTP_CLIENT_BASIC_HPP
#define SERVICES_HTTP_CLIENT_BASIC_HPP

#include "infra/timer/Timer.hpp"
#include "services/network/Http.hpp"

namespace services
{
    class HttpClientBasic
        : protected services::HttpClientObserver
        , private services::HttpClientObserverFactory
    {
    public:
        HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector);
        HttpClientBasic(infra::BoundedString url, uint16_t port, services::HttpClientConnector& httpClientConnector, infra::Duration timeoutDuration);

        void Cancel(const infra::Function<void()>& onDone);

    protected:
        void Connect();

        const infra::BoundedString& Url() const;
        virtual infra::BoundedString Path() const;
        virtual services::HttpHeaders Headers() const;

    protected:
        void Close();

        virtual void Established();
        virtual void Failed();
        virtual void TimedOut();
        virtual void Expired();

    protected:
        // Implementation of HttpClientObserver
        virtual void ClosingConnection() override;
        virtual void BodyComplete() override;

    private:
        void Timeout();
        void Expire();

    private:
        // Implementation of HttpClientObserverFactory
        virtual infra::BoundedConstString Hostname() const final;
        virtual uint16_t Port() const final;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::HttpClientObserver> client)>&& createdClientObserver) final;
        virtual void ConnectionFailed(ConnectFailReason reason) final;

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
        State state;
    };
}

#endif
