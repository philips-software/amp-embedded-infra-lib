#ifndef PROTOBUF_TEST_HELPER_ECHO_SINGLE_LOOPBACK_HPP
#define PROTOBUF_TEST_HELPER_ECHO_SINGLE_LOOPBACK_HPP

#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "protobuf/echo/Echo.hpp"
#include <deque>

namespace application
{
    class EchoSingleLoopback
        : public services::EchoOnStreams
    {
    public:
        using services::EchoOnStreams::EchoOnStreams;
        ~EchoSingleLoopback();

    protected:
        // Implementation of services::EchoOnStreams
        void RequestSendStream(std::size_t size) override;
        void AckReceived() override;

    private:
        void SendStreamFilled();
        void TryForward();
        void ReaderDone();

    private:
        std::vector<uint8_t> storage;
        infra::ByteOutputStreamWriter sendStream{ infra::ByteRange() };
        infra::AccessedBySharedPtr sendStreamAccess{
            [this]()
            {
                SendStreamFilled();
            }
        };

        infra::NotifyingSharedOptional<infra::ByteInputStreamReader> reader{
            [this]()
            {
                ReaderDone();
            }
        };

        std::deque<std::vector<uint8_t>> sending;
    };
}

#endif
