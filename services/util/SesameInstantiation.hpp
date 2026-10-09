#ifndef SERVICES_UTIL_SESAME_INSTANTIATION_HPP
#define SERVICES_UTIL_SESAME_INSTANTIATION_HPP

#include "infra/util/WithStorage.hpp"
#include "services/util/SesameCobs.hpp"
#include "services/util/SesameWindowed.hpp"

namespace main_
{
    struct Sesame
    {
    public:
        struct CobsStorageBase
        {
            CobsStorageBase(infra::BoundedVector<uint8_t>& cobsSendStorage, infra::BoundedDeque<uint8_t>& cobsReceivedMessage,
                infra::BoundedDeque<uint8_t>& windowedReceivedMessage, uint8_t windowedReceiveBuffers);

            infra::BoundedVector<uint8_t>& cobsSendMessage;
            infra::BoundedDeque<uint8_t>& cobsReceivedMessage;
            infra::BoundedDeque<uint8_t>& windowedReceivedMessage;
            uint8_t windowedReceiveBuffers;
        };

        template<std::size_t MessageSize, uint8_t SplitBuffers>
        struct CobsStorage
            : CobsStorageBase
        {
            static_assert(SplitBuffers >= 2, "Sesame requires at least 2 receive buffers");

            static constexpr std::size_t encodedMessageSize = services::SesameWindowed::bufferSizeForMessage<MessageSize, services::SesameCobs::EncodedMessageSize>;

            CobsStorage()
                : CobsStorageBase(cobsSendMessageStorage, cobsReceivedMessageStorage, windowedReceivedMessageStorage, SplitBuffers)
            {}

            infra::BoundedVector<uint8_t>::WithMaxSize<services::SesameCobs::sendBufferSize<MessageSize>> cobsSendMessageStorage;
            infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameCobs::receiveBufferSize<encodedMessageSize>> cobsReceivedMessageStorage;
            infra::BoundedDeque<uint8_t>::WithMaxSize<services::SesameWindowed::receiveBufferSize<MessageSize, SplitBuffers>> windowedReceivedMessageStorage;
        };

        template<std::size_t MessageSize, uint8_t SplitBuffers = 2>
        using WithMessageSize = infra::WithStorage<Sesame, CobsStorage<MessageSize, SplitBuffers>>;

        Sesame(CobsStorageBase& storage, hal::BufferedSerialCommunication& serialCommunication, services::SesameInitializer& initializer = services::immediatelyGranted);

        void Stop(const infra::Function<void()>& onDone);

        services::SesameCobs cobs;
        services::SesameWindowed windowed;

        infra::AutoResetFunction<void()> onStopDone;
    };
}

#endif
