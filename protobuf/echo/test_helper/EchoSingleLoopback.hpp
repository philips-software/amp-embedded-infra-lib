#ifndef PROTOBUF_TEST_HELPER_ECHO_SINGLE_LOOPBACK_HPP
#define PROTOBUF_TEST_HELPER_ECHO_SINGLE_LOOPBACK_HPP

#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "protobuf/echo/Echo.hpp"

namespace application
{
    class EchoSingleLoopback
        : public services::EchoOnStreams
    {
    public:
        using services::EchoOnStreams::EchoOnStreams;

    protected:
        //Implementation of services::EchoOnStreams
        virtual void RequestSendStream(std::size_t size) override;
        virtual void BusyServiceDone() override;

    private:
        void SendStreamFilled();

    private:
        std::vector<uint8_t> storage;
        infra::NotifyingSharedOptional<infra::ByteOutputStreamWriter> sendStream{[this]() { SendStreamFilled(); }};
    };
}

#endif
