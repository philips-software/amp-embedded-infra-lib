#include "services/util/MessageCommunicationCobs.hpp"

namespace services
{
    namespace
    {
        const uint8_t messageDelimiter = 0;
    }

    MessageCommunicationCobs::MessageCommunicationCobs(infra::BoundedVector<uint8_t>& sendStorage, infra::BoundedDeque<uint8_t>& receivedMessage, hal::BufferedSerialCommunication& serial)
        : hal::BufferedSerialCommunicationObserver(serial)
        , receivedMessage(receivedMessage)
        , sendStorage(sendStorage)
        , sendStream([this]()
              {
                  SendStreamFilled();
              })
    {}

    void MessageCommunicationCobs::RequestSendMessage(std::size_t size)
    {
        assert(sendReqestedSize == infra::none);
        sendReqestedSize = size;

        CheckReadyToSendUserData();
    }

    std::size_t MessageCommunicationCobs::MaxSendMessageSize() const
    {
        return sendStorage.max_size() - 2 - (sendStorage.max_size() - 2) / 255;
    }

    std::size_t MessageCommunicationCobs::MessageSize(std::size_t size) const
    {
        return size + size / 254 + 2;
    }

    void MessageCommunicationCobs::DataReceived()
    {
        while (receivedDataReader.Allocatable())
        {
            auto& reader = hal::BufferedSerialCommunicationObserver::Subject().Reader();
            auto data = reader.ExtractContiguousRange(std::numeric_limits<uint32_t>::max());
            if (data.empty())
                break;
            ReceivedData(data);
            reader.Rewind(reader.ConstructSaveMarker() - data.size());
            hal::BufferedSerialCommunicationObserver::Subject().AckReceived();
        }
    }

    void MessageCommunicationCobs::ReceivedData(infra::ConstByteRange& data)
    {
        while (!data.empty())
        {
            if (nextOverhead == 1)
            {
                ExtractOverhead(data);
                if (!receivedDataReader.Allocatable())
                    break;
            }
            else
                ExtractData(data);
        }
    }

    void MessageCommunicationCobs::ExtractOverhead(infra::ConstByteRange& data)
    {
        nextOverhead = data.front();

        if (nextOverhead == 0)
            MessageBoundary(data);
        else
        {
            data.pop_front();
            if (!overheadPositionIsPseudo)
                ReceivedPayload(infra::MakeByteRange(messageDelimiter));
            overheadPositionIsPseudo = nextOverhead == 255;
        }
    }

    void MessageCommunicationCobs::ExtractData(infra::ConstByteRange& data)
    {
        infra::ConstByteRange preDelimiter;
        infra::ConstByteRange postDelimiter;
        std::tie(preDelimiter, postDelimiter) = infra::FindAndSplit(infra::Head(data, nextOverhead - 1), messageDelimiter);

        if (!preDelimiter.empty())
            ForwardData(preDelimiter, data);

        if (!postDelimiter.empty())
            MessageBoundary(data);
    }

    void MessageCommunicationCobs::ForwardData(infra::ConstByteRange contents, infra::ConstByteRange& data)
    {
        ReceivedPayload(contents);
        data.pop_front(contents.size());
        nextOverhead -= static_cast<uint8_t>(contents.size());
    }

    void MessageCommunicationCobs::MessageBoundary(infra::ConstByteRange& data)
    {
        FinishMessage();
        data.pop_front();
        nextOverhead = 1;
        overheadPositionIsPseudo = true;
    }

    void MessageCommunicationCobs::ReceivedPayload(infra::ConstByteRange data)
    {
        receivedMessage.insert(receivedMessage.end(), data.begin(), data.end());
        currentMessageSize += data.size();
    }

    void MessageCommunicationCobs::FinishMessage()
    {
        auto messageSize = currentMessageSize;
        currentMessageSize = 0;

        if (messageSize != 0)
        {
            receivedDataReader.OnAllocatable([this, messageSize]()
                {
                    receivedMessage.erase(receivedMessage.begin(), receivedMessage.begin() + messageSize);
                    DataReceived();
                });
            GetObserver().ReceivedMessage(receivedDataReader.Emplace(infra::inPlace, receivedMessage, messageSize));
        }
    }

    void MessageCommunicationCobs::CheckReadyToSendUserData()
    {
        if (!sendingUserData && sendReqestedSize != infra::none)
        {
            sendStorage.clear();
            MessageCommunicationEncoded::GetObserver().SendMessageStreamAvailable(sendStream.Emplace(infra::inPlace, sendStorage, *sendReqestedSize));
            sendReqestedSize = infra::none;
        }
    }

    void MessageCommunicationCobs::SendStreamFilled()
    {
        sendingUserData = true;
        dataToSend = infra::MakeRange(sendStorage);

        if (sendingFirstPacket)
            SendFirstDelimiter();
        else
            SendOrDone();
    }

    void MessageCommunicationCobs::SendOrDone()
    {
        if (dataToSend.empty())
            SendLastDelimiter();
        else
            SendFrame();
    }

    void MessageCommunicationCobs::SendFrame()
    {
        frameSize = FindDelimiter() + 1;
        sendSizeEncoded += 1;

        hal::BufferedSerialCommunicationObserver::Subject().SendData(infra::MakeByteRange(frameSize), [this]()
            {
                --frameSize;
                if (frameSize != 0)
                    SendData(infra::Head(dataToSend, frameSize));
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

    void MessageCommunicationCobs::SendFirstDelimiter()
    {
        sendingFirstPacket = false;
        sendSizeEncoded += 1;
        hal::BufferedSerialCommunicationObserver::Subject().SendData(infra::MakeByteRange(messageDelimiter), [this]()
            {
                SendOrDone();
            });
    }

    void MessageCommunicationCobs::SendLastDelimiter()
    {
        sendSizeEncoded += 1;
        hal::BufferedSerialCommunicationObserver::Subject().SendData(infra::MakeByteRange(messageDelimiter), [this]()
            {
                MessageCommunicationEncoded::GetObserver().MessageSent(sendSizeEncoded);
                sendSizeEncoded = 0;

                sendingUserData = false;
                CheckReadyToSendUserData();
            });
    }

    void MessageCommunicationCobs::SendData(infra::ConstByteRange data)
    {
        sendSizeEncoded += data.size();
        hal::BufferedSerialCommunicationObserver::Subject().SendData(data, [this]()
            {
                SendFrameDone();
            });
    }

    uint8_t MessageCommunicationCobs::FindDelimiter() const
    {
        auto data = infra::Head(dataToSend, 254);
        return static_cast<uint8_t>(std::find(data.begin(), data.end(), messageDelimiter) - data.begin());
    }
}
