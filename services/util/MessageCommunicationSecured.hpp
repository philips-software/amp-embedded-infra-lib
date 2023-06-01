#ifndef SERVICES_MESSAGE_COMMUNICATION_SECURED_HPP
#define SERVICES_MESSAGE_COMMUNICATION_SECURED_HPP

#include "infra/stream/BoundedVectorInputStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/WithStorage.hpp"
#include "mbedtls/gcm.h"
#include "services/util/MessageCommunication.hpp"

namespace services
{
    class MessageCommunicationSecured
        : public MessageCommunication
        , private MessageCommunicationObserver
    {
    public:
        static constexpr std::size_t keySize = 16;
        static constexpr std::size_t blockSize = 16;
        using KeyType = std::array<uint8_t, keySize>;
        using IvType = std::array<uint8_t, blockSize>;

        template<std::size_t Size>
        using WithBuffers = infra::WithStorage<infra::WithStorage<MessageCommunicationSecured, infra::BoundedVector<uint8_t>::WithMaxSize<Size + blockSize>>, infra::BoundedVector<uint8_t>::WithMaxSize<Size + blockSize>>;

        MessageCommunicationSecured(infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, MessageCommunication& delegate, const KeyType& sendKey, const IvType& sendIv, const KeyType& receiveKey, const IvType& receiveIv);
        ~MessageCommunicationSecured();

        void SetSendKey(const KeyType& sendKey, const IvType& sendIv);
        void SetReceiveKey(const KeyType& receiveKey, const IvType& receiveIv);

        // Implementation of MessageCommunication
        void Initialized() override;
        void RequestSendMessage(uint16_t size) override;
        std::size_t MaxSendMessageSize() const override;

    private:
        // Implementation of MessageCommunicationObserver
        void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

        void SendMessageStreamReleased();
        void IncreaseIv(infra::ByteRange iv) const;

    private:
        class ReceiveBufferReader
            : public infra::BoundedVectorInputStreamReader
        {
        public:
            ReceiveBufferReader(const infra::BoundedVector<uint8_t>& buffer, const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader);

        private:
            infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
        };

    private:
        mbedtls_gcm_context sendContext;
        infra::BoundedVector<uint8_t>& sendBuffer;
        std::array<uint8_t, keySize> initialSendKey;
        std::array<uint8_t, blockSize> initialSendIv;
        std::array<uint8_t, blockSize> sendIv;
        infra::SharedPtr<infra::StreamWriter> sendWriter;
        infra::NotifyingSharedOptional<infra::LimitedStreamWriter::WithOutput<infra::BoundedVectorStreamWriter>> sendBufferWriter{ [this]()
            {
                SendMessageStreamReleased();
            } };
        uint16_t requestedSendSize = 0;

        mbedtls_gcm_context receiveContext;
        infra::BoundedVector<uint8_t>& receiveBuffer;
        std::array<uint8_t, keySize> initialReceiveKey;
        std::array<uint8_t, blockSize> initialReceiveIv;
        std::array<uint8_t, blockSize> receiveIv;
        infra::SharedOptional<ReceiveBufferReader> receiveBufferReader;
    };
}

#endif
