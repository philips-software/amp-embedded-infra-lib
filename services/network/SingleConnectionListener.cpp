#include "services/network/SingleConnectionListener.hpp"

namespace services
{
    SingleConnectionListener::SingleConnectionListener(ConnectionFactory& connectionFactory, uint16_t port, const Creators& creators)
        : connectionCreator(creators.connectionCreator)
        , listener(connectionFactory.Listen(port, *this))
    {}

    void SingleConnectionListener::Stop(const infra::Function<void()>& onDone)
    {
        if (connection.Allocatable())
            onDone();
        else
        {
            connection.OnAllocatable(onDone);

            if (connection)
                (*connection)->Subject().AbortAndDestroy();
        }
    }

    void SingleConnectionListener::ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address)
    {
        if (this->createdObserver != nullptr)
            this->createdObserver(nullptr);
        this->createdObserver = std::move(createdObserver);

        Stop([this]() { CreateObserver(); });
    }

    void SingleConnectionListener::CreateObserver()
    {
        connection.OnAllocatable(infra::emptyFunction);
        auto proxyPtr = connection.Emplace(connectionCreator);
        this->createdObserver(infra::MakeContainedSharedObject(**proxyPtr, proxyPtr));
    }
}
