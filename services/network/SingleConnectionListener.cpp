#include "services/network/SingleConnectionListener.hpp"

namespace services
{
    SingleConnectionListener::SingleConnectionListener(ConnectionFactory& connectionFactory, uint16_t port, const Creators& creators)
        : connectionCreator(creators.connectionCreator)
        , listener(connectionFactory.Listen(port, *this))
    {}

    void SingleConnectionListener::ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address)
    {
        if (this->createdObserver != nullptr)
            this->createdObserver(nullptr);
        this->createdObserver = std::move(createdObserver);

        if (connection.Allocatable())
            CreateObserver();
        else
        {
            connection.OnAllocatable([this]() { CreateObserver(); });

            if (connection)
                (*connection)->Subject().AbortAndDestroy();
        }
    }

    void SingleConnectionListener::CreateObserver()
    {
        connection.OnAllocatable(infra::emptyFunction);
        auto proxyPtr = connection.Emplace(connectionCreator);
        this->createdObserver(infra::MakeContainedSharedObject(**proxyPtr, proxyPtr));
    }
}
