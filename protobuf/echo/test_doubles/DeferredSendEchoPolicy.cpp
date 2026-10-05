#include "protobuf/echo/test_doubles/DeferredSendEchoPolicy.hpp"
#include "infra/util/ReallyAssert.hpp"
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

    bool DeferredSendEchoPolicy::RequestSendPending(ServiceProxy& proxy) const
    {
        return std::any_of(deferredRequests.begin(), deferredRequests.end(), [&proxy](const DeferredRequest& deferred)
            {
                return deferred.first == &proxy;
            });
    }

    void DeferredSendEchoPolicy::CancelRequestSend(ServiceProxy& proxy)
    {
        really_assert(RequestSendPending(proxy));
        deferredRequests.erase(std::remove_if(deferredRequests.begin(), deferredRequests.end(), [&proxy](const DeferredRequest& deferred)
                                   {
                                       return deferred.first == &proxy;
                                   }),
            deferredRequests.end());
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
