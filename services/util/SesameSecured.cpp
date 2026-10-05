#include "services/util/SesameSecured.hpp"
#include "infra/util/ReallyAssert.hpp"
#include <algorithm>

namespace services
{
    SesameSecured::SesameSecured(infra::BoundedVector<uint8_t>& sendBuffer, infra::BoundedVector<uint8_t>& receiveBuffer, Sesame& delegate, AesGcmEncryption& sendEncryption, AesGcmEncryption& receiveEncryption, const KeyMaterial& keyMaterial)
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
        ApplySendKey(initialSendKey, initialSendIv);
        ApplyReceiveKey(initialReceiveKey, initialReceiveIv);
    }

    void SesameSecured::SetSendKey(const KeyType& newSendKey, const IvType& newSendIv)
    {
        if (encrypting)
            pendingSendKey.emplace(newSendKey, newSendIv);
        else
            ApplySendKey(newSendKey, newSendIv);
    }

    void SesameSecured::SetReceiveKey(const KeyType& newReceiveKey, const IvType& newReceiveIv)
    {
        if (decrypting)
            pendingReceiveKey.emplace(newReceiveKey, newReceiveIv);
        else
            ApplyReceiveKey(newReceiveKey, newReceiveIv);
    }

    void SesameSecured::Initialized()
    {
        integrityCheckFailed = false;
        integrityCheckFailedTimer.Cancel();
        ++sendEpoch;
        ++receiveEpoch;
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
        ++sendEpoch;
        ++receiveEpoch;
        receiveReader = nullptr;
        waitingReceiveReader = nullptr;
        SesameObserver::Subject().Reset();
    }

    void SesameSecured::ResetReading()
    {
        ++receiveEpoch;
        SesameObserver::Subject().ResetReading();
    }

    void SesameSecured::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        if (encrypting)
        {
            waitingSendWriter = std::move(writer);
            return;
        }

        sendWriter = std::move(writer);
        GetObserver().SendMessageStreamAvailable(sendBufferWriter.Emplace(std::in_place, sendBuffer, requestedSendSize));
    }

    void SesameSecured::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        if (decrypting)
        {
            waitingReceiveReader = std::move(reader);
            return;
        }

        if (integrityCheckFailed)
        {
            // If a message with a failed integrity check is followed by another message instead of a reset,
            // then the integrity failure was not due to a truncated message
            ReportIntegrityCheckFailed();
            return;
        }

        infra::DataInputStream::WithErrorPolicy stream(*reader);

        if (stream.Available() < blockSize)
            return;

        const auto encryptedPayloadSize = stream.Available() - blockSize;
        really_assert(encryptedPayloadSize <= receiveBuffer.max_size());
        receiveBuffer.resize(encryptedPayloadSize);
        stream >> infra::MakeRange(receiveBuffer) >> infra::MakeRange(receivedMac);

        receiveReader = std::move(reader);
        decrypting = true;
        decryptingEpoch = receiveEpoch;
        receiveEncryption.Process(receiveIv, infra::MakeRange(receiveBuffer), infra::MakeRange(computedMac), [this]()
            {
                Decrypted();
            });
    }

    void SesameSecured::ApplySendKey(const KeyType& newSendKey, const IvType& newSendIv)
    {
        sendEncryption.SetEncryptKey(newSendKey);
        sendIv = newSendIv;
    }

    void SesameSecured::ApplyReceiveKey(const KeyType& newReceiveKey, const IvType& newReceiveIv)
    {
        receiveEncryption.SetDecryptKey(newReceiveKey);
        receiveIv = newReceiveIv;
    }

    void SesameSecured::ApplyPendingSendKey()
    {
        if (pendingSendKey != std::nullopt)
        {
            ApplySendKey(pendingSendKey->first, pendingSendKey->second);
            pendingSendKey.reset();
        }
    }

    void SesameSecured::ApplyPendingReceiveKey()
    {
        if (pendingReceiveKey != std::nullopt)
        {
            ApplyReceiveKey(pendingReceiveKey->first, pendingReceiveKey->second);
            pendingReceiveKey.reset();
        }
    }

    void SesameSecured::SendMessageStreamReleased()
    {
        const auto payloadSize = sendBuffer.size();
        sendBuffer.resize(payloadSize + blockSize);

        encrypting = true;
        encryptingEpoch = sendEpoch;
        sendEncryption.Process(sendIv, infra::Head(infra::MakeRange(sendBuffer), payloadSize), infra::Tail(infra::MakeRange(sendBuffer), blockSize), [this]()
            {
                Encrypted();
            });
    }

    void SesameSecured::Encrypted()
    {
        encrypting = false;

        if (encryptingEpoch == sendEpoch)
        {
            infra::DataOutputStream::WithErrorPolicy stream(*sendWriter);
            stream << infra::MakeRange(sendBuffer);
            IncreaseIv(sendIv);
        }

        sendBuffer.clear();
        sendWriter = nullptr;
        ApplyPendingSendKey();

        if (waitingSendWriter != nullptr)
            SendMessageStreamAvailable(std::exchange(waitingSendWriter, nullptr));
    }

    void SesameSecured::Decrypted()
    {
        decrypting = false;

        if (decryptingEpoch != receiveEpoch)
        {
            receiveReader = nullptr;
            receiveBuffer.clear();
            ApplyPendingReceiveKey();
        }
        else if (!MacIsValid())
        {
            receiveReader = nullptr;
            receiveBuffer.clear();
            ApplyPendingReceiveKey();
            IntegrityCheckFailedOnReceive();
        }
        else
        {
            IncreaseIv(receiveIv);
            ApplyPendingReceiveKey();
            Sesame::GetObserver().ReceivedMessage(receiveBufferReader.Emplace(receiveBuffer, std::exchange(receiveReader, nullptr)));
        }

        if (waitingReceiveReader != nullptr)
            ReceivedMessage(std::exchange(waitingReceiveReader, nullptr));
    }

    bool SesameSecured::MacIsValid() const
    {
        uint32_t numSame = 0;
        for (std::size_t i = 0; i != computedMac.size(); ++i)
            numSame += computedMac[i] == receivedMac[i];

        return numSame == computedMac.size();
    }

    void SesameSecured::IntegrityCheckFailedOnReceive()
    {
        integrityCheckFailed = true;
        integrityCheckFailedTimer.Start(std::chrono::seconds(1), [this]()
            {
                // If a message with a failed integrity check that message could have
                // been a truncated message. However, if that message was terminated by a 0,
                // but not followed by an init message, then this was not a truncated message
                ReportIntegrityCheckFailed();
            });
    }

    void SesameSecured::IncreaseIv(infra::ByteRange iv) const
    {
        for (auto i = iv.begin() + iv.size(); i != iv.begin(); --i)
            if (++*std::prev(i) != 0)
                break;
    }

    void SesameSecured::ReportIntegrityCheckFailed()
    {
        IntegritySubject::NotifyObservers([](auto& observer)
            {
                observer.IntegrityCheckFailed();
            });
    }

    SesameSecured::ReceiveBufferReader::ReceiveBufferReader(const infra::BoundedVector<uint8_t>& buffer, const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader)
        : infra::BoundedVectorInputStreamReader(buffer)
        , reader(reader)
    {}
}
