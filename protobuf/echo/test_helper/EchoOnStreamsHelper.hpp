#ifndef PROTOBUF_TEST_HELPER_ECHO_TEST_HELPER_HPP
#define PROTOBUF_TEST_HELPER_ECHO_TEST_HELPER_HPP

#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "protobuf/echo/Echo.hpp"

namespace application
{
    class EchoOnStreamsHelper
        : public services::EchoOnStreams
    {
    public:
        template<std::size_t Size>
            using WithStorage = infra::WithStorage<EchoOnStreamsHelper, infra::BoundedVector<uint8_t>::WithMaxSize<Size>>;

        using services::EchoOnStreams::EchoOnStreams;

        explicit EchoOnStreamsHelper(infra::BoundedVector<uint8_t>& storage);

    protected:
        //Implementation of services::EchoOnStreams
        virtual void RequestSendStream(std::size_t size) override;
        virtual void BusyServiceDone() override;

    private:
        void SendStreamFilled();

    private:
        std::size_t messageSize = 0;
        infra::BoundedVector<uint8_t>& storage;
        infra::NotifyingSharedOptional<infra::LimitedStreamWriter::WithOutput<infra::BoundedVectorStreamWriter>> sendStream;
    };
}

#endif
