#ifndef SERVICES_MESSAGE_COMMUNICATION_SYMMETRIC_KEY_DISTRIBUTION_HPP
#define SERVICES_MESSAGE_COMMUNICATION_SYMMETRIC_KEY_DISTRIBUTION_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "services/util/MessageCommunicationSecured.hpp"

namespace services
{
    class MessageCommunicationSymmetricKeyDistribution
        : public MessageCommunication
        , private MessageCommunicationObserver
    {
    public:
        MessageCommunicationSymmetricKeyDistribution(MessageCommunication& delegate, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        // Implementation of MessageCommunication
        virtual void Initialized() override;
        virtual void RequestSendMessage(uint16_t size) override;
        virtual std::size_t MaxSendMessageSize() const override;

    private:
        // Implementation of MessageCommunicationObserver
        virtual void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

    private:
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        bool updatingKey = true;
    };
}

#endif
