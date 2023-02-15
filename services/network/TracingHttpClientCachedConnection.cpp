#include "services/network/TracingHttpClientCachedConnection.hpp"

namespace services
{
    TracingHttpClientCachedConnectionConnector::TracingHttpClientCachedConnectionConnector(HttpClientConnector& delegate, const Sha256& hasher, Tracer& tracer, infra::Duration disconnectTimeout)
        : HttpClientCachedConnectionConnector(delegate, hasher, disconnectTimeout)
        , tracer(tracer)
    {}

    void TracingHttpClientCachedConnectionConnector::RetargetConnection()
    {
        tracer.Trace() << "HttpClientCachedConnectionConnector::RetargetConnection";

        HttpClientCachedConnectionConnector::RetargetConnection();
    }

    void TracingHttpClientCachedConnectionConnector::Connect()
    {
        tracer.Trace() << "HttpClientCachedConnectionConnector::Connect";

        HttpClientCachedConnectionConnector::Connect();
    }

    void TracingHttpClientCachedConnectionConnector::DetachingObserver()
    {
        tracer.Trace() << "HttpClientCachedConnectionConnector::DetachingObserver";

        HttpClientCachedConnectionConnector::DetachingObserver();
    }

    void TracingHttpClientCachedConnectionConnector::DisconnectTimeout()
    {
        tracer.Trace() << "HttpClientCachedConnectionConnector::DisconnectTimeout";

        HttpClientCachedConnectionConnector::DisconnectTimeout();
    }
}
