#include "services/network/Connection.hpp"

namespace services
{
    void Connection::SwitchObserver(const infra::SharedPtr<ConnectionObserver>& newObserver)
    {
        this->observer = newObserver;
    }

    void Connection::SetOwnership(const infra::SharedPtr<void>& owner, const infra::SharedPtr<ConnectionObserver>& observer)
    {
        this->owner = owner;
        this->observer = observer;
    }

    void Connection::ResetOwnership()
    {
        if (observer != nullptr)
        {
            // Someone may be keeping the observer alive, so give it the opportunity to close any open send/receive streams,
            // and detach it so that the owner is not observed anymore
            observer->ClosingConnection();
            observer->Detach();
        }
        observer = nullptr;
        owner = nullptr;
    }

    infra::SharedPtr<ConnectionObserver> Connection::Observer() const
    {
        return observer;
    }
}
