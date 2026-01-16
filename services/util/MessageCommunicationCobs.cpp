#include "services/util/MessageCommunicationCobs.hpp"
#include "infra/stream/BoundedVectorInputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"

namespace services
{
    namespace
    {
        const uint8_t messageDelimiter = 0;
    }

    MessageCommunicationCobs::MessageCommunicationCobs(infra::BoundedVector<uint8_t>& sendStorage, infra::BoundedVector<uint8_t>& receivedMessage, hal::SerialCommunication& serial)
        : serial(serial)
        , receivedMessage(receivedMessage)
        , sendStorage(sendStorage)
        , sendStream([this]()
              {
                  SendStreamFilled();
              })
    {
        serial.ReceiveData([this](infra::ConstByteRange data)
            {
                ReceivedDataOnInterrupt(data);
            });
    }

    MessageCommunicationCobs::~MessageCommunicationCobs()
    {
        serial.ReceiveData([](infra::ConstByteRange data) {});
    }

    infra::SharedPtr<infra::StreamWriter> MessageCommunicationCobs::SendMessageStream(uint16_t size, const infra::Function<void(uint16_t size)>& onSent)
    {
        sendStorage.clear();
        this->onSent = onSent;
        return sendStream.Emplace(std::in_place, sendStorage, size);
    }

    std::size_t MessageCommunicationCobs::MaxSendMessageSize() const
    {
        return sendStorage.max_size();
    }

    void MessageCommunicationCobs::ReceivedDataOnInterrupt(infra::ConstByteRange data)
    {
        while (!data.empty())
        {
            if (nextOverhead == 1)
                ExtractOverheadOnInterrupt(data);
            else
                ExtractDataOnInterrupt(data);
        }
    }

    void MessageCommunicationCobs::ExtractOverheadOnInterrupt(infra::ConstByteRange& data)
    {
        nextOverhead = data.front();

        if (nextOverhead == 0)
            StartNewMessageOnInterrupt(data);
        else
        {
            data.pop_front();
            if (!overheadPositionIsPseudo)
                ReceivedPayloadOnInterrupt(infra::MakeByteRange(messageDelimiter));
            overheadPositionIsPseudo = nextOverhead == 255;
        }
    }

    void MessageCommunicationCobs::ExtractDataOnInterrupt(infra::ConstByteRange& data)
    {
        infra::ConstByteRange preDelimiter;
        infra::ConstByteRange postDelimiter;
        std::tie(preDelimiter, postDelimiter) = infra::FindAndSplit(infra::Head(data, nextOverhead - 1), messageDelimiter);

        if (!preDelimiter.empty())
            ForwardDataOnInterrupt(preDelimiter, data);

        if (!postDelimiter.empty())
            StartNewMessageOnInterrupt(data);
    }

    void MessageCommunicationCobs::ForwardDataOnInterrupt(infra::ConstByteRange contents, infra::ConstByteRange& data)
    {
        ReceivedPayloadOnInterrupt(contents);
        data.pop_front(contents.size());
        nextOverhead -= static_cast<uint8_t>(contents.size());
    }

    void MessageCommunicationCobs::StartNewMessageOnInterrupt(infra::ConstByteRange& data)
    {
        MessageBoundaryOnInterrupt();
        data.pop_front();
        nextOverhead = 1;
        overheadPositionIsPseudo = true;
    }

    void MessageCommunicationCobs::ReceivedPayloadOnInterrupt(infra::ConstByteRange data)
    {
        receivedMessage.insert(receivedMessage.end(), data.begin(), data.end());
        currentMessageSize += data.size();
    }

    void MessageCommunicationCobs::MessageBoundaryOnInterrupt()
    {
        auto messageSize = currentMessageSize;
        currentMessageSize = 0;

        if (messageSize != 0)
        {
            infra::LimitedStreamReader::WithInput<infra::BoundedVectorInputStreamReader> reader(std::in_place, receivedMessage, messageSize);
            GetObserver().ReceivedMessageOnInterrupt(reader);
            receivedMessage.erase(receivedMessage.begin(), receivedMessage.begin() + messageSize);
        }
    }

    void MessageCommunicationCobs::SendStreamFilled()
    {
        dataToSend = infra::MakeRange(sendStorage);
        serial.SendData(infra::MakeByteRange(messageDelimiter), [this]()
            {
                SendOrDone();
            });
    }

    void MessageCommunicationCobs::SendOrDone()
    {
        if (dataToSend.empty())
            serial.SendData(infra::MakeByteRange(messageDelimiter), [this]()
                {
                    onSent(static_cast<uint16_t>(sendStorage.size()));
                });
        else
            SendFrame();
    }

    void MessageCommunicationCobs::SendFrame()
    {
        frameSize = FindDelimiter() + 1;

        serial.SendData(infra::MakeByteRange(frameSize), [this]()
            {
                --frameSize;
                if (frameSize != 0)
                    serial.SendData(infra::Head(dataToSend, frameSize), [this]()
                        {
                            SendFrameDone();
                        });
                else
                    SendFrameDone();
            });
    }

    void MessageCommunicationCobs::SendFrameDone()
    {
        if (frameSize == 254)
            dataToSend = infra::DiscardHead(dataToSend, 254);
        else
            dataToSend = infra::DiscardHead(dataToSend, frameSize);

        if (dataToSend.empty() || frameSize == 254)
            SendOrDone();
        else
        {
            dataToSend.pop_front();
            SendFrame();
        }
    }

    uint8_t MessageCommunicationCobs::FindDelimiter() const
    {
        auto data = infra::Head(dataToSend, 254);
        return static_cast<uint8_t>(std::find(data.begin(), data.end(), messageDelimiter) - data.begin());
    }
}
