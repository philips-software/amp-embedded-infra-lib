#include "services/network/SingleConnectionListener.hpp"

namespace services
{
    SingleConnectionListener::SingleConnectionListener(ConnectionFactory& connectionFactory, uint16_t port, const Creators& creators)
        : connectionCreator(creators.connectionCreator)
        , listener(connectionFactory.Listen(port, *this))
    {}

    void SingleConnectionListener::Stop(const infra::Function<void()>& onDone)
    {
        listener = nullptr;
        Stop(onDone, true);
    }

    void SingleConnectionListener::ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address)
    {
        if (this->createdObserver != nullptr)
            this->createdObserver(nullptr);
        this->createdObserver = std::move(createdObserver);
        this->address = address;

        Stop([this]() { CreateObserver(); }, false);
    }

    void SingleConnectionListener::Stop(const infra::Function<void()>& onDone, bool force)
    {
        if (connection.Allocatable())
            onDone();
        else
        {
            connection.OnAllocatable(onDone);

            if (connection)
            {
                if (force)
                    (*connection)->Abort();
                else
                    (*connection)->Close();
            }
        }
    }

    void SingleConnectionListener::CreateObserver()
    {
        connection.OnAllocatable(infra::emptyFunction);
        auto proxyPtr = connection.Emplace(connectionCreator, address);
        this->createdObserver(infra::MakeContainedSharedObject(**proxyPtr, proxyPtr));
    }
}
