#ifndef SERVICES_MESSAGE_COMMUNICATION_COBS_HPP
#define SERVICES_MESSAGE_COMMUNICATION_COBS_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/util/Sesame.hpp"

namespace services
{
    class SesameCobs
        : public SesameEncoded
        , private hal::BufferedSerialCommunicationObserver
    {
    public:
        template<std::size_t MaxMessageSize>
        using WithMaxMessageSize = infra::WithStorage<infra::WithStorage<SesameCobs,
                                                          infra::BoundedVector<uint8_t>::WithMaxSize<MaxMessageSize>>,
            infra::BoundedDeque<uint8_t>::WithMaxSize<MaxMessageSize + MaxMessageSize / 254 + 2>>;

        SesameCobs(infra::BoundedVector<uint8_t>& sendStorage, infra::BoundedDeque<uint8_t>& receivedMessage, hal::BufferedSerialCommunication& serial);

        // Implementation of Sesame
        void RequestSendMessage(std::size_t size) override;
        std::size_t MaxSendMessageSize() const override;
        virtual std::size_t MessageSize(std::size_t size) const override;

    private:
        // Implementation of BufferedSerialCommunicationObserver
        void DataReceived() override;

    private:
        void ReceivedData(infra::ConstByteRange& data);
        void ExtractOverhead(infra::ConstByteRange& data);
        void ExtractData(infra::ConstByteRange& data);
        void ForwardData(infra::ConstByteRange contents, infra::ConstByteRange& data);
        void MessageBoundary(infra::ConstByteRange& data);
        void ReceivedPayload(infra::ConstByteRange data);
        void FinishMessage();

        void CheckReadyToSendUserData();
        void SendStreamFilled();
        void SendOrDone();
        void SendFrame();
        void SendFrameDone();
        void SendFirstDelimiter();
        void SendLastDelimiter();
        void SendData(infra::ConstByteRange data);
        uint8_t FindDelimiter() const;

    private:
        infra::Optional<std::size_t> sendReqestedSize;
        bool sendingFirstPacket = true;
        bool sendingUserData = false;
        std::size_t sendSizeEncoded = 0;
        uint8_t nextOverhead = 1;
        bool overheadPositionIsPseudo = true;
        infra::BoundedDeque<uint8_t>& receivedMessage;
        std::size_t receiveSizeEncoded = 0;
        std::size_t currentMessageSize = 0;
        infra::NotifyingSharedOptional<infra::LimitedStreamReaderWithRewinding::WithInput<infra::BoundedDequeInputStreamReader>> receivedDataReader;

        infra::BoundedVector<uint8_t>& sendStorage;
        infra::NotifyingSharedOptional<infra::LimitedStreamWriter::WithOutput<infra::BoundedVectorStreamWriter>> sendStream;
        infra::ConstByteRange dataToSend;
        uint8_t frameSize;
    };
}

#endif
