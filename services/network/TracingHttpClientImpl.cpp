#include "services/network/TracingHttpClientImpl.hpp"

namespace services
{
    TracingHttpClientImpl::TracingHttpClientImpl(infra::BoundedString& headerBuffer, infra::BoundedConstString hostname, services::Tracer& tracer)
        : HttpClientImpl(headerBuffer, hostname)
        , tracer(tracer)
    {}

    void TracingHttpClientImpl::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        tracer.Trace() << "HttpClientImpl::SendStreamAvailable; sending request:" << infra::endl;
        request->Write(tracer.Trace());
        HttpClientImpl::SendStreamAvailable(std::move(writer));
    }

    void TracingHttpClientImpl::DataReceived()
    {
        tracer.Trace() << "HttpClientImpl::DataReceived; received response:" << infra::endl;

        auto reader = ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);
        tracer.Trace();

        while (!stream.Empty())
        {
            auto range = stream.ContiguousRange();

            while (!range.empty())
            {
                infra::BoundedString::WithStorage<256> dummy(256, 'x');
                if (dummy.size() > range.size())
                    dummy.resize(range.size());

                range.pop_back(dummy.size());

                tracer.Continue() << dummy;
            }
        }

        reader = nullptr;

        HttpClientImpl::DataReceived();
    }

    void TracingHttpClientImpl::Connected()
    {
        tracer.Trace() << "HttpClientImpl::Connected";
        HttpClientImpl::Connected();
    }

    void TracingHttpClientImpl::ClosingConnection()
    {
        tracer.Trace() << "HttpClientImpl::ClosingConnection";
        HttpClientImpl::ClosingConnection();
    }
}
