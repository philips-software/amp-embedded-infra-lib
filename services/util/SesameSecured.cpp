#include "services/util/SesameSecured.hpp"
#include <algorithm>

namespace services
{
    sesame_security::SymmetricKeyFile GenerateSymmetricKeys(hal::SynchronousRandomDataGenerator& randomDataGenerator)
    {
        sesame_security::SymmetricKeyFile keys;

        keys.sendBySelf.key.resize(keys.sendBySelf.key.max_size());
        randomDataGenerator.GenerateRandomData(infra::MakeRange(keys.sendBySelf.key));
        keys.sendBySelf.iv.resize(keys.sendBySelf.iv.max_size());
        randomDataGenerator.GenerateRandomData(infra::MakeRange(keys.sendBySelf.iv));
        keys.sendByOther.key.resize(keys.sendByOther.key.max_size());
        randomDataGenerator.GenerateRandomData(infra::MakeRange(keys.sendByOther.key));
        keys.sendByOther.iv.resize(keys.sendByOther.iv.max_size());
        randomDataGenerator.GenerateRandomData(infra::MakeRange(keys.sendByOther.iv));

        return keys;
    }

    sesame_security::SymmetricKeyFile ReverseDirection(const sesame_security::SymmetricKeyFile& keys)
    {
        return { keys.sendByOther, keys.sendBySelf };
    }

    SesameSecured::SesameSecured(AesGcmEncryption& sendEncryption, AesGcmEncryption& receiveEncryption, infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, Sesame& delegate,
        const KeyMaterial& keyMaterial)
        : SesameObserver(delegate)
        , sendEncryption(sendEncryption)
        , receiveEncryption(receiveEncryption)
        , sendBuffer(sendBuffer)
        , initialSendKey(keyMaterial.sendKey)
        , initialSendIv(keyMaterial.sendIv)
        , receiveBuffer(receiveBuffer)
        , initialReceiveKey(keyMaterial.receiveKey)
        , initialReceiveIv(keyMaterial.receiveIv)
    {
        SetSendKey(initialSendKey, initialSendIv);
        SetReceiveKey(initialReceiveKey, initialReceiveIv);
    }

    SesameSecured::SesameSecured(AesGcmEncryption& sendEncryption, AesGcmEncryption& receiveEncryption, infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, Sesame& delegate, const sesame_security::SymmetricKeyFile& keyMaterial)
        : SesameSecured(sendEncryption, receiveEncryption, sendBuffer, receiveBuffer, delegate, ConvertKeyMaterial(keyMaterial))
    {}

    void SesameSecured::SetSendKey(const KeyType& newSendKey, const IvType& newSendIv)
    {
        sendEncryption.EncryptWithKey(newSendKey);
        sendIv = newSendIv;
    }

    void SesameSecured::SetReceiveKey(const KeyType& newReceiveKey, const IvType& newReceiveIv)
    {
        receiveEncryption.DecryptWithKey(newReceiveKey);
        receiveIv = newReceiveIv;
    }

    void SesameSecured::Initialized()
    {
        SetSendKey(initialSendKey, initialSendIv);
        SetReceiveKey(initialReceiveKey, initialReceiveIv);
        GetObserver().Initialized();
    }

    void SesameSecured::RequestSendMessage(std::size_t size)
    {
        really_assert(size <= MaxSendMessageSize());
        requestedSendSize = size;
        SesameObserver::Subject().RequestSendMessage(size + blockSize);
    }

    std::size_t SesameSecured::MaxSendMessageSize() const
    {
        return std::min(SesameObserver::Subject().MaxSendMessageSize(), sendBuffer.max_size()) - blockSize;
    }

    void SesameSecured::Reset()
    {
        SesameObserver::Subject().Reset();
    }

    void SesameSecured::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        sendWriter = std::move(writer);
        GetObserver().SendMessageStreamAvailable(sendBufferWriter.Emplace(infra::inPlace, sendBuffer, requestedSendSize));
    }

    void SesameSecured::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        if (stream.Available() < blockSize)
            return;

        receiveBuffer.clear();

        receiveEncryption.Start(receiveIv);

        while (stream.Available() != blockSize)
        {
            infra::BoundedVector<uint8_t>::WithMaxSize<blockSize> encrypted;
            encrypted.resize(std::min(encrypted.max_size(), stream.Available() - blockSize));
            stream >> infra::MakeRange(encrypted);

            receiveBuffer.resize(receiveBuffer.size() + encrypted.size());
            std::size_t processedSize = receiveEncryption.Update(infra::MakeRange(encrypted), infra::Tail(infra::MakeRange(receiveBuffer), encrypted.size()));
            receiveBuffer.resize(receiveBuffer.size() - encrypted.size() + processedSize);
        }

        std::array<uint8_t, blockSize> computedMac;
        receiveEncryption.Finish(infra::ByteRange(), computedMac);

        std::array<uint8_t, blockSize> receivedMac;
        stream >> infra::MakeRange(receivedMac);

        uint32_t numSame = 0;
        for (auto i = 0; i != computedMac.size(); ++i)
            numSame += computedMac[i] == receivedMac[i];

        if (numSame != computedMac.size())
            return;

        IncreaseIv(receiveIv);

        Sesame::GetObserver().ReceivedMessage(receiveBufferReader.Emplace(receiveBuffer, reader));
    }

    void SesameSecured::SendMessageStreamReleased()
    {
        sendEncryption.Start(sendIv);
        auto processedSize = sendEncryption.Update(infra::MakeRange(sendBuffer), infra::MakeRange(sendBuffer));
        sendBuffer.resize(sendBuffer.size() + blockSize);
        auto moreProcessedSize = sendEncryption.Finish(infra::DiscardTail(infra::DiscardHead(infra::MakeRange(sendBuffer), processedSize), blockSize), infra::Tail(infra::MakeRange(sendBuffer), blockSize));
        really_assert(processedSize + moreProcessedSize + blockSize == sendBuffer.size());

        infra::DataOutputStream::WithErrorPolicy stream(*sendWriter);
        stream << infra::MakeRange(sendBuffer);
        sendBuffer.clear();
        IncreaseIv(sendIv);
        sendWriter = nullptr;
    }

    void SesameSecured::IncreaseIv(infra::ByteRange iv) const
    {
        for (auto i = iv.begin() + iv.size() / 2; i != iv.begin(); --i)
            if (++*std::prev(i) != 0)
                break;
    }

    SesameSecured::ReceiveBufferReader::ReceiveBufferReader(const infra::BoundedVector<uint8_t>& buffer, const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader)
        : infra::BoundedVectorInputStreamReader(buffer)
        , reader(reader)
    {}

#ifdef EMIL_USE_MBEDTLS
    SesameSecured::WithCryptoMbedTls::WithCryptoMbedTls(infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, Sesame& delegate, const KeyMaterial& keyMaterial)
        : SesameSecured(detail::SesameSecuredMbedTlsEncryptors::sendEncryption, detail::SesameSecuredMbedTlsEncryptors::receiveEncryption, sendBuffer, receiveBuffer, delegate, keyMaterial)
    {}

    SesameSecured::WithCryptoMbedTls::WithCryptoMbedTls(infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, Sesame& delegate, const sesame_security::SymmetricKeyFile& keyMaterial)
        : SesameSecured(detail::SesameSecuredMbedTlsEncryptors::sendEncryption, detail::SesameSecuredMbedTlsEncryptors::receiveEncryption, sendBuffer, receiveBuffer, delegate, keyMaterial)
    {}
#endif

    SesameSecured::KeyMaterial ConvertKeyMaterial(const sesame_security::SymmetricKeyFile& keyMaterial)
    {
        SesameSecured::KeyMaterial result;

        infra::Copy(infra::MakeRange(keyMaterial.sendBySelf.key), infra::MakeRange(result.sendKey));
        infra::Copy(infra::MakeRange(keyMaterial.sendBySelf.iv), infra::MakeRange(result.sendIv));
        infra::Copy(infra::MakeRange(keyMaterial.sendByOther.key), infra::MakeRange(result.receiveKey));
        infra::Copy(infra::MakeRange(keyMaterial.sendByOther.iv), infra::MakeRange(result.receiveIv));

        return result;
    }
}
