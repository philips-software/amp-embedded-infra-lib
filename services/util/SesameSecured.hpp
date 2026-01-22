#ifndef SERVICES_SESAME_SECURED_HPP
#define SERVICES_SESAME_SECURED_HPP

#include "generated/echo/SesameSecurity.pb.hpp"
#include "infra/stream/BoundedVectorInputStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/util/Sesame.hpp"
#include "services/util/SesameCrypto.hpp"
#ifdef EMIL_USE_MBEDTLS
#include "services/util/SesameCryptoMbedTls.hpp"
#endif

namespace services
{
    sesame_security::SymmetricKeyFile GenerateSymmetricKeys(hal::SynchronousRandomDataGenerator& randomDataGenerator);
    sesame_security::SymmetricKeyFile ReverseDirection(const sesame_security::SymmetricKeyFile& keys);

    class IntegritySubject;

    class IntegrityObserver
        : public infra::Observer<IntegrityObserver, IntegritySubject>
    {
    public:
        using infra::Observer<IntegrityObserver, IntegritySubject>::Observer;

        virtual void IntegrityCheckFailed() = 0;
    };

    class IntegritySubject
        : public infra::Subject<IntegrityObserver>
    {};

    class SesameSecured
        : public Sesame
        , public IntegritySubject
        , private SesameObserver
    {
    public:
        static constexpr std::size_t keySize = 16;
        static constexpr std::size_t blockSize = 16;
        using KeyType = std::array<uint8_t, keySize>;
        using IvType = std::array<uint8_t, blockSize>;

        template<std::size_t Size>
        static constexpr std::size_t encodedMessageSize = Size + blockSize;

        struct KeyMaterial
        {
            KeyType sendKey;
            IvType sendIv;
            KeyType receiveKey;
            IvType receiveIv;
        };

#ifdef EMIL_USE_MBEDTLS
        struct WithCryptoMbedTls;
#endif

        SesameSecured(AesGcmEncryption& sendEncryption, AesGcmEncryption& receiveEncryption, infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, Sesame& delegate, const KeyMaterial& keyMaterial);
        SesameSecured(AesGcmEncryption& sendEncryption, AesGcmEncryption& receiveEncryption, infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, Sesame& delegate, const sesame_security::SymmetricKeyFile& keyMaterial);

        void SetSendKey(const KeyType& newSendKey, const IvType& newSendIv);
        void SetReceiveKey(const KeyType& newReceiveKey, const IvType& newReceiveIv);

        // Implementation of Sesame
        void Initialized() override;
        void RequestSendMessage(std::size_t size) override;
        std::size_t MaxSendMessageSize() const override;
        void Reset() override;

    private:
        // Implementation of SesameObserver
        void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

        void ActivateSendKey();
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
        AesGcmEncryption& sendEncryption;
        AesGcmEncryption& receiveEncryption;
        infra::BoundedVector<uint8_t>& sendBuffer;
        std::array<uint8_t, keySize> initialSendKey;
        std::array<uint8_t, blockSize> initialSendIv;
        std::array<uint8_t, blockSize> sendIv;
        infra::SharedPtr<infra::StreamWriter> sendWriter;
        infra::NotifyingSharedOptional<infra::LimitedStreamWriter::WithOutput<infra::BoundedVectorStreamWriter>> sendBufferWriter{ [this]()
            {
                SendMessageStreamReleased();
            } };
        std::size_t requestedSendSize = 0;

        infra::BoundedVector<uint8_t>& receiveBuffer;
        std::array<uint8_t, keySize> initialReceiveKey;
        std::array<uint8_t, blockSize> initialReceiveIv;
        std::array<uint8_t, blockSize> receiveIv;
        infra::SharedOptional<ReceiveBufferReader> receiveBufferReader;
        bool integrityCheckFailed = false;
    };

#ifdef EMIL_USE_MBEDTLS
    namespace detail
    {
        struct SesameSecuredMbedTlsEncryptors
        {
            AesGcmEncryptionMbedTls sendEncryption;
            AesGcmEncryptionMbedTls receiveEncryption;
        };
    }

    struct SesameSecured::WithCryptoMbedTls
        : private detail::SesameSecuredMbedTlsEncryptors
        , public SesameSecured
    {
        template<std::size_t Size>
        using WithBuffers = infra::WithStorage<infra::WithStorage<WithCryptoMbedTls, infra::BoundedVector<uint8_t>::WithMaxSize<encodedMessageSize<Size>>>, infra::BoundedVector<uint8_t>::WithMaxSize<encodedMessageSize<Size>>>;

        WithCryptoMbedTls(infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, Sesame& delegate, const KeyMaterial& keyMaterial);
        WithCryptoMbedTls(infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, Sesame& delegate, const sesame_security::SymmetricKeyFile& keyMaterial);
    };
#endif

    SesameSecured::KeyMaterial ConvertKeyMaterial(const sesame_security::SymmetricKeyFile& keyMaterial);
}

#endif
