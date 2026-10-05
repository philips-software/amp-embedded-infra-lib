#ifndef SERVICES_SESAME_SECURED_HPP
#define SERVICES_SESAME_SECURED_HPP

#include "infra/stream/BoundedVectorInputStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/synchronous_util/SynchronousSesameSecured.hpp"
#include "services/util/Sesame.hpp"
#include "services/util/SesameCrypto.hpp"
#include <optional>

namespace services
{
    class SesameSecured
        : public Sesame
        , public IntegritySubject
        , private SesameObserver
    {
    public:
        static constexpr std::size_t keySize = 16;
        static constexpr std::size_t blockSize = 16;
        static constexpr std::size_t ivSize = 12;
        using KeyType = std::array<uint8_t, keySize>;
        using IvType = std::array<uint8_t, ivSize>;

        template<std::size_t Size>
        static constexpr std::size_t encodedMessageSize = Size + blockSize;

        struct KeyMaterial
        {
            KeyType sendKey;
            IvType sendIv;
            KeyType receiveKey;
            IvType receiveIv;
        };

        template<std::size_t Size>
        struct alignas(4) AlignedBuffer
            : infra::BoundedVector<uint8_t>::WithMaxSize<Size>
        {};

        template<std::size_t Size>
        using WithBuffers = infra::WithStorage<infra::WithStorage<SesameSecured, AlignedBuffer<encodedMessageSize<Size>>>, AlignedBuffer<encodedMessageSize<Size>>>;

        SesameSecured(infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, Sesame& delegate, AesGcmEncryption& sendEncryption, AesGcmEncryption& receiveEncryption, const KeyMaterial& keyMaterial);

        void SetSendKey(const KeyType& newSendKey, const IvType& newSendIv);
        void SetReceiveKey(const KeyType& newReceiveKey, const IvType& newReceiveIv);

        // Implementation of Sesame
        void Initialized() override;
        void RequestSendMessage(std::size_t size) override;
        std::size_t MaxSendMessageSize() const override;
        void Reset() override;
        void ResetReading() override;

    private:
        // Implementation of SesameObserver
        void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

        void ApplySendKey(const KeyType& newSendKey, const IvType& newSendIv);
        void ApplyReceiveKey(const KeyType& newReceiveKey, const IvType& newReceiveIv);
        void ApplyPendingSendKey();
        void ApplyPendingReceiveKey();
        void SendMessageStreamReleased();
        void Encrypted();
        void Decrypted();
        bool MacIsValid() const;
        void IntegrityCheckFailedOnReceive();
        void IncreaseIv(infra::ByteRange iv) const;
        void ReportIntegrityCheckFailed();

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
        AesGcmEncryption& sendEncryption;
        AesGcmEncryption& receiveEncryption;

        infra::BoundedVector<uint8_t>& sendBuffer;
        KeyType initialSendKey;
        IvType initialSendIv;
        IvType sendIv;
        std::optional<std::pair<KeyType, IvType>> pendingSendKey;
        infra::SharedPtr<infra::StreamWriter> sendWriter;
        infra::SharedPtr<infra::StreamWriter> waitingSendWriter;
        infra::NotifyingSharedOptional<infra::LimitedStreamWriter::WithOutput<infra::BoundedVectorStreamWriter>> sendBufferWriter{ [this]()
            {
                SendMessageStreamReleased();
            } };
        std::size_t requestedSendSize = 0;
        bool encrypting = false;
        uint32_t sendEpoch = 0;
        uint32_t encryptingEpoch = 0;

        infra::BoundedVector<uint8_t>& receiveBuffer;
        KeyType initialReceiveKey;
        IvType initialReceiveIv;
        IvType receiveIv;
        std::optional<std::pair<KeyType, IvType>> pendingReceiveKey;
        infra::SharedPtr<infra::StreamReaderWithRewinding> receiveReader;
        infra::SharedPtr<infra::StreamReaderWithRewinding> waitingReceiveReader;
        std::array<uint8_t, blockSize> receivedMac{};
        std::array<uint8_t, blockSize> computedMac{};
        bool decrypting = false;
        uint32_t receiveEpoch = 0;
        uint32_t decryptingEpoch = 0;
        infra::SharedOptional<ReceiveBufferReader> receiveBufferReader;
        bool integrityCheckFailed = false;
        infra::TimerSingleShot integrityCheckFailedTimer;
    };
}

#endif
