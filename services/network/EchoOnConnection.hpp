#ifndef SERVICES_ECHO_ON_CONNECTION_HPP
#define SERVICES_ECHO_ON_CONNECTION_HPP

#include "protobuf/echo/Echo.hpp"
#include "services/network/Connection.hpp"

namespace services
{
    class EchoOnConnection
        : public EchoOnStreams
        , public ConnectionObserver
    {
    public:
        using EchoOnStreams::EchoOnStreams;

        // Implementation of ConnectionObserver
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void DataReceived() override;

    protected:
        // Implementation of EchoOnStreams
        void RequestSendStream(std::size_t size) override;

    private:
        infra::SharedPtr<infra::StreamReaderWithRewinding> readerPtr;
        infra::AccessedBySharedPtr access{
            [this]()
            {
                if (ConnectionObserver::IsAttached())
                    ConnectionObserver::Subject().AckReceived();
            }
        };
    };
}

#endif
