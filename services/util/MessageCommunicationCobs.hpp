#ifndef SERVICES_MESSAGE_COMMUNICATION_COBS_HPP
#define SERVICES_MESSAGE_COMMUNICATION_COBS_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/util/MessageCommunication.hpp"

namespace services
{
    class MessageCommunicationCobs
        : public MessageCommunicationReceiveOnInterrupt
    {
    public:
        template<std::size_t MaxMessageSize>
            using WithMaxMessageSize = infra::WithStorage<infra::WithStorage<MessageCommunicationCobs,
                infra::BoundedVector<uint8_t>::WithMaxSize<MaxMessageSize>>,
                infra::BoundedVector<uint8_t>::WithMaxSize<MaxMessageSize + MaxMessageSize / 254 + 3>>;

        MessageCommunicationCobs(infra::BoundedVector<uint8_t>& sendStorage, infra::BoundedVector<uint8_t>& receivedMessage, hal::SerialCommunication& serial);

        // Implementation of MessageCommunication
        virtual infra::SharedPtr<infra::StreamWriter> SendMessageStream(uint16_t size, const infra::Function<void(uint16_t size)>& onSent) override;
        virtual std::size_t MaxSendMessageSize() const override;

    private:
        void ReceivedDataOnInterrupt(infra::ConstByteRange data);
        void ExtractOverheadOnInterrupt(infra::ConstByteRange& data);
        void ExtractDataOnInterrupt(infra::ConstByteRange& data);
        void ForwardDataOnInterrupt(infra::ConstByteRange contents, infra::ConstByteRange& data);
        void StartNewMessageOnInterrupt(infra::ConstByteRange& data);
        void ReceivedPayloadOnInterrupt(infra::ConstByteRange data);
        void MessageBoundaryOnInterrupt();

        void SendStreamFilled();
        void SendOrDone();
        void SendFrame();
        void SendFrameDone();
        uint8_t FindDelimiter() const;

    private:
        hal::SerialCommunication& serial;

        uint8_t nextOverhead = 1;
        bool overheadPositionIsPseudo = true;
        infra::BoundedVector<uint8_t>& receivedMessage;
        std::size_t currentMessageSize = 0;

        infra::BoundedVector<uint8_t>& sendStorage;
        infra::NotifyingSharedOptional<infra::LimitedStreamWriter::WithOutput<infra::BoundedVectorStreamWriter>> sendStream;
        infra::Function<void(uint16_t size)> onSent;
        infra::ConstByteRange dataToSend;
        uint8_t frameSize;
    };
}

#endif
