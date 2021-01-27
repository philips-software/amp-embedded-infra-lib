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

    bool ClaimableResource::ClaimsPending() const
    {
        return !pendingClaims.empty();
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

    void ClaimableResource::AddClaim(ClaimerBase& claimer, bool urgent)
    {
        EnqueueClaimer(claimer, urgent);

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

    void ClaimableResource::ReplaceClaim(ClaimerBase& claimerOld, ClaimerBase& claimerNew)
    {
        if (currentClaim == &claimerOld)
            currentClaim = &claimerNew;

        for (auto index = pendingClaims.begin(); index != pendingClaims.end(); ++index)
            if (&*index == &claimerOld)
            {
                pendingClaims.insert(index, claimerNew);
                pendingClaims.erase(claimerOld);
                return;
            }
    }

    void ClaimableResource::EnqueueClaimer(ClaimerBase& claimer, bool urgent)
    {
        if (urgent)
            pendingClaims.push_front(claimer);
        else
            pendingClaims.push_back(claimer);
    }

    void ClaimableResource::DequeueClaimer(ClaimerBase& claimer)
    {
        pendingClaims.erase(claimer);
    }

    ClaimableResource::ClaimerBase::ClaimerBase(ClaimableResource& resource)
        : resource(resource)
    {}

    ClaimableResource::ClaimerBase::ClaimerBase(ClaimerBase&& other)
        : resource(other.resource)
        , isGranted(other.isGranted)
        , isQueued(other.isQueued)
    {
        other.isGranted = false;
        other.isQueued = false;

        if (isQueued || isGranted)
            resource.ReplaceClaim(other, *this);
    }

    ClaimableResource::ClaimerBase& ClaimableResource::ClaimerBase::operator=(ClaimerBase&& other)
    {
        assert(&resource == &other.resource);
        ReleaseAllClaims();

        isGranted = other.isGranted;
        isQueued = other.isQueued;

        other.isGranted = false;
        other.isQueued = false;

        if (isQueued || isGranted)
            resource.ReplaceClaim(other, *this);

        return *this;
    }

    ClaimableResource::ClaimerBase::~ClaimerBase()
    {
        ReleaseAllClaims();
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

    bool ClaimableResource::ClaimerBase::IsQueued() const
    {
        return isQueued;
    }

    void ClaimableResource::ClaimerBase::ReleaseAllClaims()
    {
        if (isGranted)
            ClaimerBase::Release();

        if (isQueued)
            ClaimerBase::Release();
    }
}
