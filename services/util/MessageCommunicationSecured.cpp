#include "services/util/MessageCommunicationSecured.hpp"
#include <algorithm>

namespace services
{
    MessageCommunicationSecured::MessageCommunicationSecured(infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, MessageCommunication& delegate,
        const KeyType& sendKey, const IvType& sendIv, const KeyType& receiveKey, const IvType& receiveIv)
        : MessageCommunicationObserver(delegate)
        , sendBuffer(sendBuffer)
        , sendIv(sendIv)
        , receiveBuffer(receiveBuffer)
        , receiveIv(receiveIv)
    {
        mbedtls_gcm_init(&sendContext);
        mbedtls_gcm_init(&receiveContext);

        mbedtls_gcm_setkey(&sendContext, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<const unsigned char*>(sendKey.data()), sendKey.size() * 8);          //NOSONAR
        mbedtls_gcm_setkey(&receiveContext, MBEDTLS_CIPHER_ID_AES, reinterpret_cast<const unsigned char*>(receiveKey.data()), receiveKey.size() * 8); //NOSONAR
    }

    MessageCommunicationSecured::~MessageCommunicationSecured()
    {
        mbedtls_gcm_free(&receiveContext);
        mbedtls_gcm_free(&sendContext);
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

    void MessageCommunicationSecured::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        sendWriter = std::move(writer);
        GetObserver().SendMessageStreamAvailable(sendBufferWriter.Emplace(infra::inPlace, sendBuffer, requestedSendSize));
    }

    void MessageCommunicationSecured::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        if (stream.Available() < blockSize)
            return;

        receiveBuffer.clear();
        really_assert(mbedtls_gcm_starts(&receiveContext, MBEDTLS_GCM_DECRYPT, reinterpret_cast<const unsigned char*>(receiveIv.data()), receiveIv.size()) == 0);

        while (stream.Available() != blockSize)
        {
            infra::BoundedVector<uint8_t>::WithMaxSize<blockSize> encrypted;
            encrypted.resize(std::min(encrypted.max_size(), stream.Available() - blockSize));
            stream >> infra::MakeRange(encrypted);

            receiveBuffer.resize(receiveBuffer.size() + encrypted.size());
            std::size_t processedSize = 0;
            really_assert(mbedtls_gcm_update(&receiveContext, encrypted.data(), encrypted.size(), receiveBuffer.data() + receiveBuffer.size() - encrypted.size(), receiveBuffer.size(), &processedSize) == 0);
            receiveBuffer.resize(receiveBuffer.size() - encrypted.size() + processedSize);
        }

        auto startSize = receiveBuffer.size();
        std::size_t processedSize = 0;
        receiveBuffer.resize(receiveBuffer.max_size());
        std::array<uint8_t, blockSize> computedMac;
        really_assert(mbedtls_gcm_finish(&receiveContext, receiveBuffer.data() + startSize, receiveBuffer.size() - startSize, &processedSize, reinterpret_cast<unsigned char*>(computedMac.data()), computedMac.size()) == 0);
        receiveBuffer.resize(startSize + processedSize);

        std::array<uint8_t, blockSize> receivedMac;
        stream >> infra::MakeRange(receivedMac);

        if (computedMac != receivedMac)
            return;

        MessageCommunication::GetObserver().ReceivedMessage(receiveBufferReader.Emplace(receiveBuffer, reader));
        IncreaseIv(receiveIv);
    }

    void MessageCommunicationSecured::SendMessageStreamReleased()
    {
        really_assert(mbedtls_gcm_starts(&sendContext, MBEDTLS_GCM_ENCRYPT, reinterpret_cast<const unsigned char*>(sendIv.data()), sendIv.size()) == 0);

        std::size_t processedSize = 0;
        really_assert(mbedtls_gcm_update(&sendContext, sendBuffer.data(), sendBuffer.size(), sendBuffer.data(), sendBuffer.size(), &processedSize) == 0);
        sendBuffer.resize(sendBuffer.size() + blockSize);

        std::size_t moreProcessedSize = 0;
        really_assert(mbedtls_gcm_finish(&sendContext, sendBuffer.data() + processedSize, sendBuffer.size() - processedSize - blockSize, &moreProcessedSize, sendBuffer.data() + sendBuffer.size() - blockSize, blockSize) == 0);
        really_assert(processedSize + moreProcessedSize + blockSize == sendBuffer.size());

        infra::DataOutputStream::WithErrorPolicy stream(*sendWriter);
        stream << infra::MakeRange(sendBuffer);
        sendBuffer.clear();
        IncreaseIv(sendIv);
        sendWriter = nullptr;
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
