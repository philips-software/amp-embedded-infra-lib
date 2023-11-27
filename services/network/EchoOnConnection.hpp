#ifndef SERVICES_ECHO_ON_CONNECTION_HPP
#define SERVICES_ECHO_ON_CONNECTION_HPP

#include "infra/stream/LimitedInputStream.hpp"
#include "infra/util/SharedOptional.hpp"
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
        void AckReceived() override;

    private:
        struct LimitedReader
        {
            LimitedReader(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader);

            infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
            infra::LimitedStreamReaderWithRewinding limitedReader;
        };

        bool delayReceived = false;
        infra::NotifyingSharedOptional<LimitedReader> reader{
            [this]()
            {
                if (delayReceived)
                {
                    delayReceived = false;
                    if (ConnectionObserver::IsAttached())
                        DataReceived();
                }
            }
        };
    };
}

#endif
