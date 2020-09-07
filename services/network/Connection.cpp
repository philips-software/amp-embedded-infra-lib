#include "services/network/Connection.hpp"

namespace services
{
    void ConnectionObserver::Close()
    {
        Subject().CloseAndDestroy();
    }

    void ConnectionObserver::Abort()
    {
        Subject().AbortAndDestroy();
    }

    void ConnectionWithHostnameDecorator::RequestSendStream(std::size_t sendSize)
    {
        return ConnectionObserver::Subject().RequestSendStream(sendSize);
    }

    std::size_t ConnectionWithHostnameDecorator::MaxSendStreamSize() const
    {
        return ConnectionObserver::Subject().MaxSendStreamSize();
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> ConnectionWithHostnameDecorator::ReceiveStream()
    {
        return ConnectionObserver::Subject().ReceiveStream();
    }

    void ConnectionWithHostnameDecorator::AckReceived()
    {
        ConnectionObserver::Subject().AckReceived();
    }

    void ConnectionWithHostnameDecorator::CloseAndDestroy()
    {
        ConnectionObserver::Subject().CloseAndDestroy();
    }

    void ConnectionWithHostnameDecorator::AbortAndDestroy()
    {
        ConnectionObserver::Subject().AbortAndDestroy();
    }

    void ConnectionWithHostnameDecorator::SetHostname(infra::BoundedConstString hostname)
    {
        static_cast<ConnectionWithHostname&>(ConnectionObserver::Subject()).SetHostname(hostname);
    }

    void ConnectionWithHostnameDecorator::Attach(const infra::SharedPtr<ConnectionObserver>& observer)
    {
        ConnectionWithHostname::Attach(observer);

        if (aborting)
            Observer().Abort();
        else if (closing)
            Observer().Close();
    }

    void ConnectionWithHostnameDecorator::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& streamWriter)
    {
        Connection::Observer().SendStreamAvailable(std::move(streamWriter));
    }

    void ConnectionWithHostnameDecorator::DataReceived()
    {
        Connection::Observer().DataReceived();
    }

    void ConnectionWithHostnameDecorator::Detaching()
    {
        ConnectionWithHostname::Detach();
    }

    void ConnectionWithHostnameDecorator::Close()
    {
        if (Connection::IsAttached())
            Observer().Close();
        else
            closing = true;
    }

    void ConnectionWithHostnameDecorator::Abort()
    {
        if (Connection::IsAttached())
            Observer().Abort();
        else
            aborting = true;
    }
}
