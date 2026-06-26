#ifndef SERVICES_UTIL_SESAME_INSTANTIATION_HPP
#define SERVICES_UTIL_SESAME_INSTANTIATION_HPP

#include "services/util/SesameCobs.hpp"
#include "services/util/SesameWindowed.hpp"

namespace main_
{
    struct Sesame
    {
    public:
        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        struct WithMessageSize;

        struct CobsStorageBase
        {
            CobsStorageBase(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage,
                infra::BoundedDeque<uint8_t>& windowedReceivedMessage, uint8_t windowedReceiveBuffers);

            infra::BoundedVector<uint8_t>& cobsSendStorage;
            infra::BoundedDeque<uint8_t>& cobsReceivedMessage;
            infra::BoundedDeque<uint8_t>& windowedReceivedMessage;
            uint8_t windowedReceiveBuffers;
        };

        template<std::size_t MessageSize, uint8_t SplitBuffers>
        struct CobsStorage
            : CobsStorageBase
        {
            static_assert(SplitBuffers >= 2, "Sesame requires at least 2 receive buffers");

            static constexpr std::size_t encodedMessageSize = services::SesameWindowed::bufferSizeForMessage<MessageSize, SplitBuffers, services::SesameCobs::EncodedMessageSize>;

            CobsStorage()
                : CobsStorageBase(cobsSendStorage, cobsReceivedMessage, windowedReceivedMessage, SplitBuffers)
            {}

            infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameCobs::sendBufferSize<MessageSize>> cobsSendStorage;
            infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameCobs::receiveBufferSize<encodedMessageSize>> cobsReceivedMessage;
            infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameCobs::receiveBufferSize<encodedMessageSize>> windowedReceivedMessage;
        };

        Sesame(CobsStorageBase& storage, hal::BufferedSerialCommunication& serialCommunication);

        void Stop(const infra::Function<void()>& onDone);

        services::SesameCobs cobs;
        services::SesameWindowed windowed;

        infra::AutoResetFunction<void()> onStopDone;
    };

    template<std::size_t MessageSize, uint8_t SplitBuffers>
    struct Sesame::WithMessageSize
        : private CobsStorage<MessageSize, SplitBuffers>
        , Sesame
    {
        WithMessageSize(hal::BufferedSerialCommunication& serialCommunication)
            : Sesame(static_cast<CobsStorageBase&>(*this), serialCommunication)
        {}
    };
}

#endif
