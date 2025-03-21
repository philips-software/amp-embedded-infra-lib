#include "services/util/SesameSecured.hpp"
#include <algorithm>

namespace services
{
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

    void SesameSecured::SetNextSendKey(const KeyType& nextSendKey, const IvType& nextSendIv)
    {
        nextKeys = { nextSendKey, nextSendIv };

        if (sendWriter == nullptr)
            ActivateSendKey();
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

    void SesameSecured::SetSendKey(const KeyType& sendKey, const IvType& sendIv)
    {
        sendEncryption.EncryptWithKey(sendKey);
        this->sendIv = sendIv;
    }

    void SesameSecured::ActivateSendKey()
    {
        SetSendKey(nextKeys->first, nextKeys->second);
        nextKeys = infra::none;
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
        std::size_t processedSize = receiveEncryption.Finish(infra::ByteRange(), computedMac);

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

        if (nextKeys)
            ActivateSendKey();
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
#endif
}
