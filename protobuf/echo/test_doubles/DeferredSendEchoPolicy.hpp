#ifndef PROTOBUF_TEST_DEFERRED_SEND_ECHO_POLICY_HPP
#define PROTOBUF_TEST_DEFERRED_SEND_ECHO_POLICY_HPP

#include "protobuf/echo/Echo.hpp"
#include <utility>
#include <vector>

namespace services
{
    class DeferredSendEchoPolicy
        : public EchoPolicy
    {
    public:
        using DeferredRequest = std::pair<ServiceProxy*, infra::Function<void(ServiceProxy& proxy)>>;

        void RequestSend(ServiceProxy& proxy, const infra::Function<void(ServiceProxy& proxy)>& onRequest) override;

        void StartDeferring();
        void GrantDeferredRequests();
        const std::vector<DeferredRequest>& DeferredRequests() const;

    private:
        bool deferring = false;
        std::vector<DeferredRequest> deferredRequests;
    };
}

#endif
