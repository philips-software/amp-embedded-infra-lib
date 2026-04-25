#ifndef SERVICES_UTIL_SESAME_INSTANTIATION_HPP
#define SERVICES_UTIL_SESAME_INSTANTIATION_HPP

#include "services/util/SesameCobs.hpp"
#include "services/util/SesameWindowed.hpp"

namespace main_
{
    struct Sesame
    {
    public:
        template<std::size_t MessageSize>
        struct WithMessageSize;

        Sesame(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage,
            infra::BoundedDeque<uint8_t>& windowedReceivedMessage, hal::BufferedSerialCommunication& serialCommunication);

        void Stop(const infra::Function<void()>& onDone);

        services::SesameCobs cobs;
        services::SesameWindowed windowed;

        infra::AutoResetFunction<void()> onStopDone;

        template<std::size_t MessageSize>
        struct CobsStorage
        {
            static constexpr std::size_t encodedMessageSize = services::SesameWindowed::bufferSizeForMessage<MessageSize, services::SesameCobs::EncodedMessageSize>;

            infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameCobs::sendBufferSize<MessageSize>> cobsSendStorage;
            infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameCobs::receiveBufferSize<encodedMessageSize>> cobsReceivedMessage;
            infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameCobs::receiveBufferSize<encodedMessageSize>> windowedReceivedMessage;
        };
    };

    template<std::size_t MessageSize>
    struct Sesame::WithMessageSize
        : private Sesame::CobsStorage<MessageSize>
        , Sesame
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication)
            : Sesame(this->cobsSendStorage, this->cobsReceivedMessage, this->windowedReceivedMessage, serialCommunication)
        {}
    };
}

#endif
