#ifndef SERVICES_MESSAGE_COMMUNICATION_COBS_HPP
#define SERVICES_MESSAGE_COMMUNICATION_COBS_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/stream/BoundedVectorInputStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/util/MessageCommunication.hpp"

namespace services
{
    class MessageCommunicationCobs
        : public MessageCommunication
        , private hal::BufferedSerialCommunicationObserver
    {
    public:
        template<std::size_t MaxMessageSize>
        using WithMaxMessageSize = infra::WithStorage<infra::WithStorage<MessageCommunicationCobs,
                                                          infra::BoundedVector<uint8_t>::WithMaxSize<MaxMessageSize>>,
            infra::BoundedVector<uint8_t>::WithMaxSize<MaxMessageSize + MaxMessageSize / 254 + 3>>;

        MessageCommunicationCobs(infra::BoundedVector<uint8_t>& sendStorage, infra::BoundedVector<uint8_t>& receivedMessage, hal::BufferedSerialCommunication& serial);

        // Implementation of MessageCommunication
        void RequestSendMessage(uint16_t size) override;
        std::size_t MaxSendMessageSize() const override;

    private:
        // Implementation of BufferedSerialCommunicationObserver
        void DataReceived() override;

    private:
        void ReceivedData(infra::ConstByteRange& data);
        void ExtractOverhead(infra::ConstByteRange& data);
        void ExtractData(infra::ConstByteRange& data);
        void ForwardData(infra::ConstByteRange contents, infra::ConstByteRange& data);
        void StartNewMessage(infra::ConstByteRange& data);
        void ReceivedPayload(infra::ConstByteRange data);
        void MessageBoundary();

        void CheckReadyToSendUserData();
        void SendStreamFilled();
        void SendOrDone();
        void SendFrame();
        void SendFrameDone();
        uint8_t FindDelimiter() const;

    private:
        infra::Optional<uint16_t> sendReqestedSize;
        bool sendingUserData = false;
        uint8_t nextOverhead = 1;
        bool overheadPositionIsPseudo = true;
        infra::BoundedVector<uint8_t>& receivedMessage;
        std::size_t currentMessageSize = 0;
        infra::NotifyingSharedOptional<infra::LimitedStreamReaderWithRewinding::WithInput<infra::BoundedVectorInputStreamReader>> receivedDataReader;

        infra::BoundedVector<uint8_t>& sendStorage;
        infra::NotifyingSharedOptional<infra::LimitedStreamWriter::WithOutput<infra::BoundedVectorStreamWriter>> sendStream;
        infra::ConstByteRange dataToSend;
        uint8_t frameSize;
    };
}

#endif
