#ifndef SERVICES_TRACING_HTTP_CLIENT_CACHED_CONNECTION_HPP
#define SERVICES_TRACING_HTTP_CLIENT_CACHED_CONNECTION_HPP

#include "services/network/HttpClientCachedConnection.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingHttpClientCachedConnectionConnector
        : public HttpClientCachedConnectionConnector
    {
    public:
        TracingHttpClientCachedConnectionConnector(HttpClientConnector& delegate, const Sha256& hasher, Tracer& tracer, infra::Duration disconnectTimeout = std::chrono::minutes(1));

        void RetargetConnection() override;
        void Connect() override;
        void DetachingObserver() override;
        void DisconnectTimeout() override;

    private:
        Tracer& tracer;
    };
}

#endif
