#ifndef SERVICES_MESSAGE_COMMUNICATION_MOCK_HPP
#define SERVICES_MESSAGE_COMMUNICATION_MOCK_HPP

#include "services/util/MessageCommunication.hpp"
#include "gmock/gmock.h"

namespace services
{

    class MessageCommunicationMock
        : public MessageCommunication
    {
    public:
        MOCK_METHOD(void, RequestSendMessage, (uint16_t size), (override));
        MOCK_METHOD(std::size_t, MaxSendMessageSize, (), (const, override));
    };

    class MessageCommunicationObserverMock
        : public MessageCommunicationObserver
    {
    public:
        using MessageCommunicationObserver::MessageCommunicationObserver;

        MOCK_METHOD(void, Initialized, (), (override));
        MOCK_METHOD(void, SendMessageStreamAvailable, (infra::SharedPtr<infra::StreamWriter> && writer), (override));
        MOCK_METHOD(void, ReceivedMessage, (infra::SharedPtr<infra::StreamReaderWithRewinding> && reader), (override));
    };
}

#endif
