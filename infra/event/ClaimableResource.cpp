#include "infra/event/ClaimableResource.hpp"
#include "infra/event/EventDispatcher.hpp"
#include <cassert>

namespace infra
{
    ClaimableResource::~ClaimableResource()
    {
        assert(currentClaim == nullptr);
        assert(pendingClaims.empty());
    }

    void ClaimableResource::ReEvaluateClaim()
    {
        if (currentClaim == nullptr && !pendingClaims.empty())
        {
            currentClaim = &pendingClaims.front();
            pendingClaims.pop_front();
            currentClaim->ClaimGranted();
        }
    }

    void ClaimableResource::AddClaim(ClaimerBase& claimer)
    {
        EnqueueClaimer(claimer);

        if (currentClaim == nullptr)
            infra::EventDispatcher::Instance().Schedule([this]() { ReEvaluateClaim(); });
    }

    void ClaimableResource::RemoveClaim(ClaimerBase& claimer)
    {
        if (currentClaim == &claimer)
        {
            currentClaim = nullptr;
            infra::EventDispatcher::Instance().Schedule([this]() { ReEvaluateClaim(); });
        }
        else if (!pendingClaims.empty())
            DequeueClaimer(claimer);
    }

    void ClaimableResource::EnqueueClaimer(ClaimerBase& claimer)
    {
        pendingClaims.push_back(claimer);
    }

    void ClaimableResource::DequeueClaimer(ClaimerBase& claimer)
    {
        pendingClaims.erase(claimer);
    }

    ClaimableResource::ClaimerBase::ClaimerBase(ClaimableResource& resource)
        : resource(resource)
    {}

    ClaimableResource::ClaimerBase::~ClaimerBase()
    {
        if (isGranted)
            ClaimerBase::Release();

        if (isQueued)
            ClaimerBase::Release();
    }

    void ClaimableResource::ClaimerBase::Release()
    {
        if (!isGranted)
            isQueued = false;

        isGranted = false;
        resource.RemoveClaim(*this);
    }

    bool ClaimableResource::ClaimerBase::IsClaimed() const
    {
        return isGranted;
    }
}
