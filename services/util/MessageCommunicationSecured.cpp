#include "services/util/MessageCommunicationSecured.hpp"
#include "mbedtls/version.h"
#include <algorithm>

namespace services
{
    MessageCommunicationSecured::MessageCommunicationSecured(infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, MessageCommunication& delegate,
        const KeyType& sendKey, const IvType& sendIv, const KeyType& receiveKey, const IvType& receiveIv)
        : MessageCommunicationObserver(delegate)
        , sendBuffer(sendBuffer)
        , initialSendKey(sendKey)
        , initialSendIv(sendIv)
        , receiveBuffer(receiveBuffer)
        , initialReceiveKey(receiveKey)
        , initialReceiveIv(receiveIv)
    {
        mbedtls_gcm_init(&sendContext);
        mbedtls_gcm_init(&receiveContext);

        SetSendKey(initialSendKey, initialSendIv);
        SetReceiveKey(initialReceiveKey, initialReceiveIv);
    }

    MessageCommunicationSecured::~MessageCommunicationSecured()
    {
        mbedtls_gcm_free(&receiveContext);
        mbedtls_gcm_free(&sendContext);
    }

    void MessageCommunicationSecured::SetNextSendKey(const KeyType& sendKey, const IvType& sendIv)
    {
        nextKeys = { sendKey, sendIv };

        if (sendWriter == nullptr)
            ActivateSendKey();
    }

    void MessageCommunicationSecured::SetReceiveKey(const KeyType& receiveKey, const IvType& receiveIv)
    {
        mbedtls_gcm_setkey(&receiveContext, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<const unsigned char*>(receiveKey.data()), receiveKey.size() * 8); //NOSONAR
        this->receiveIv = receiveIv;
    }

    void MessageCommunicationSecured::Initialized()
    {
        SetSendKey(initialSendKey, initialSendIv);
        SetReceiveKey(initialReceiveKey, initialReceiveIv);
        GetObserver().Initialized();
    }

    void MessageCommunicationSecured::RequestSendMessage(uint16_t size)
    {
        really_assert(size <= MaxSendMessageSize());
        requestedSendSize = size;
        MessageCommunicationObserver::Subject().RequestSendMessage(size + blockSize);
    }

    std::size_t MessageCommunicationSecured::MaxSendMessageSize() const
    {
        return std::min(MessageCommunicationObserver::Subject().MaxSendMessageSize(), sendBuffer.max_size()) - blockSize;
    }

    void MessageCommunicationSecured::SetSendKey(const KeyType& sendKey, const IvType& sendIv)
    {
        mbedtls_gcm_setkey(&sendContext, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<const unsigned char*>(sendKey.data()), sendKey.size() * 8); //NOSONAR
        this->sendIv = sendIv;
    }

    void MessageCommunicationSecured::ActivateSendKey()
    {
        SetSendKey(nextKeys->first, nextKeys->second);
        nextKeys = std::nullopt;
    }

    void MessageCommunicationSecured::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        sendWriter = std::move(writer);
        GetObserver().SendMessageStreamAvailable(sendBufferWriter.emplace(std::in_place, sendBuffer, requestedSendSize));
    }

    void MessageCommunicationSecured::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        if (stream.Available() < blockSize)
            return;

        receiveBuffer.clear();

#if MBEDTLS_VERSION_MAJOR < 3
        really_assert(mbedtls_gcm_starts(&receiveContext, MBEDTLS_GCM_DECRYPT, reinterpret_cast<const unsigned char*>(receiveIv.data()), receiveIv.size(), nullptr, 0) == 0);
#else
        really_assert(mbedtls_gcm_starts(&receiveContext, MBEDTLS_GCM_DECRYPT, reinterpret_cast<const unsigned char*>(receiveIv.data()), receiveIv.size()) == 0);
#endif

        while (stream.Available() != blockSize)
        {
            infra::BoundedVector<uint8_t>::WithMaxSize<blockSize> encrypted;
            encrypted.resize(std::min(encrypted.max_size(), stream.Available() - blockSize));
            stream >> infra::MakeRange(encrypted);

            receiveBuffer.resize(receiveBuffer.size() + encrypted.size());
#if MBEDTLS_VERSION_MAJOR < 3
            really_assert(mbedtls_gcm_update(&receiveContext, encrypted.size(), encrypted.data(), receiveBuffer.data() + receiveBuffer.size() - encrypted.size()) == 0);
#else
            std::size_t processedSize = 0;
            really_assert(mbedtls_gcm_update(&receiveContext, encrypted.data(), encrypted.size(), receiveBuffer.data() + receiveBuffer.size() - encrypted.size(), receiveBuffer.size(), &processedSize) == 0);
            receiveBuffer.resize(receiveBuffer.size() - encrypted.size() + processedSize);
#endif
        }

        std::array<uint8_t, blockSize> computedMac;
#if MBEDTLS_VERSION_MAJOR < 3
        really_assert(mbedtls_gcm_finish(&receiveContext, reinterpret_cast<unsigned char*>(computedMac.data()), computedMac.size()) == 0);
#else
        std::size_t processedSize = 0;
        really_assert(mbedtls_gcm_finish(&receiveContext, nullptr, 0, &processedSize, reinterpret_cast<unsigned char*>(computedMac.data()), computedMac.size()) == 0);
#endif

        std::array<uint8_t, blockSize> receivedMac;
        stream >> infra::MakeRange(receivedMac);

        if (computedMac != receivedMac)
            return;

        IncreaseIv(receiveIv);
        MessageCommunication::GetObserver().ReceivedMessage(receiveBufferReader.emplace(receiveBuffer, reader));
    }

    void MessageCommunicationSecured::SendMessageStreamReleased()
    {
#if MBEDTLS_VERSION_MAJOR < 3
        really_assert(mbedtls_gcm_starts(&sendContext, MBEDTLS_GCM_ENCRYPT, reinterpret_cast<const unsigned char*>(sendIv.data()), sendIv.size(), nullptr, 0) == 0);
#else
        really_assert(mbedtls_gcm_starts(&sendContext, MBEDTLS_GCM_ENCRYPT, reinterpret_cast<const unsigned char*>(sendIv.data()), sendIv.size()) == 0);
#endif

#if MBEDTLS_VERSION_MAJOR < 3
        really_assert(mbedtls_gcm_update(&sendContext, sendBuffer.size(), sendBuffer.data(), sendBuffer.data()) == 0);
#else
        std::size_t processedSize = 0;
        really_assert(mbedtls_gcm_update(&sendContext, sendBuffer.data(), sendBuffer.size(), sendBuffer.data(), sendBuffer.size(), &processedSize) == 0);
#endif

        sendBuffer.resize(sendBuffer.size() + blockSize);

#if MBEDTLS_VERSION_MAJOR < 3
        really_assert(mbedtls_gcm_finish(&sendContext, sendBuffer.data() + sendBuffer.size() - blockSize, blockSize) == 0);
#else
        std::size_t moreProcessedSize = 0;
        really_assert(mbedtls_gcm_finish(&sendContext, sendBuffer.data() + processedSize, sendBuffer.size() - processedSize - blockSize, &moreProcessedSize, sendBuffer.data() + sendBuffer.size() - blockSize, blockSize) == 0);
        really_assert(processedSize + moreProcessedSize + blockSize == sendBuffer.size());
#endif

        infra::DataOutputStream::WithErrorPolicy stream(*sendWriter);
        stream << infra::MakeRange(sendBuffer);
        sendBuffer.clear();
        IncreaseIv(sendIv);
        sendWriter = nullptr;

        if (nextKeys)
            ActivateSendKey();
    }

    void MessageCommunicationSecured::IncreaseIv(infra::ByteRange iv) const
    {
        for (auto i = iv.begin() + iv.size() / 2; i != iv.begin(); --i)
            if (++*std::prev(i) != 0)
                break;
    }

    MessageCommunicationSecured::ReceiveBufferReader::ReceiveBufferReader(const infra::BoundedVector<uint8_t>& buffer, const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader)
        : infra::BoundedVectorInputStreamReader(buffer)
        , reader(reader)
    {}
}
