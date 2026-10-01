#include "protobuf/echo/test_doubles/DeferredSendEchoPolicy.hpp"

namespace services
{
    void DeferredSendEchoPolicy::RequestSend(ServiceProxy& proxy, const infra::Function<void(ServiceProxy& proxy)>& onRequest)
    {
        if (deferring)
            deferredRequests.emplace_back(&proxy, onRequest);
        else
            onRequest(proxy);
    }

    void DeferredSendEchoPolicy::StartDeferring()
    {
        deferring = true;
    }

    void DeferredSendEchoPolicy::GrantDeferredRequests()
    {
        deferring = false;

        auto requests = std::move(deferredRequests);
        deferredRequests.clear();

        for (const auto& [proxy, onRequest] : requests)
            onRequest(*proxy);
    }

    const std::vector<DeferredSendEchoPolicy::DeferredRequest>& DeferredSendEchoPolicy::DeferredRequests() const
    {
        return deferredRequests;
    }
}
