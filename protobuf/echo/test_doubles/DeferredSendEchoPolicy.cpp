#include "protobuf/echo/test_doubles/DeferredSendEchoPolicy.hpp"
#include <algorithm>

namespace services
{
    void DeferredSendEchoPolicy::RequestSend(ServiceProxy& proxy, const infra::Function<void(ServiceProxy& proxy)>& onRequest)
    {
        if (deferring)
            deferredRequests.emplace_back(&proxy, onRequest);
        else
            onRequest(proxy);
    }

    bool DeferredSendEchoPolicy::CancelRequestSend(ServiceProxy& proxy)
    {
        auto request = std::find_if(deferredRequests.begin(), deferredRequests.end(), [&proxy](const DeferredRequest& deferred)
            {
                return deferred.first == &proxy;
            });

        if (request == deferredRequests.end())
            return false;

        deferredRequests.erase(request);
        return true;
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
