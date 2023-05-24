#include "services/util/MessageCommunicationSymmetricKeyDistribution.hpp"

namespace services
{
    MessageCommunicationSymmetricKeyDistribution::MessageCommunicationSymmetricKeyDistribution(MessageCommunication& delegate, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : randomDataGenerator(randomDataGenerator)
    {}

    void MessageCommunicationSymmetricKeyDistribution::Initialized()
    {
        updatingKey = true;
        GetObserver().Initialized();
    }

    void MessageCommunicationSymmetricKeyDistribution::RequestSendMessage(uint16_t size)
    {
        MessageCommunicationObserver::Subject().RequestSendMessage(size);
    }

    std::size_t MessageCommunicationSymmetricKeyDistribution::MaxSendMessageSize() const
    {
        return MessageCommunicationObserver::Subject().MaxSendMessageSize();
    }

    void MessageCommunicationSymmetricKeyDistribution::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
    }

    void MessageCommunicationSymmetricKeyDistribution::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {

    }
}
