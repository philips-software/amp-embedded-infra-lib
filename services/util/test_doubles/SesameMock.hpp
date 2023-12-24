#ifndef SERVICES_MESSAGE_COMMUNICATION_MOCK_HPP
#define SERVICES_MESSAGE_COMMUNICATION_MOCK_HPP

#include "services/util/Sesame.hpp"
#include "gmock/gmock.h"

namespace services
{
    class SesameMock
        : public Sesame
    {
    public:
        MOCK_METHOD(void, RequestSendMessage, (std::size_t size), (override));
        MOCK_METHOD(std::size_t, MaxSendMessageSize, (), (const, override));
    };

    class SesameObserverMock
        : public SesameObserver
    {
    public:
        using SesameObserver::SesameObserver;

        MOCK_METHOD(void, Initialized, (), (override));
        MOCK_METHOD(void, SendMessageStreamAvailable, (infra::SharedPtr<infra::StreamWriter> && writer), (override));
        MOCK_METHOD(void, ReceivedMessage, (infra::SharedPtr<infra::StreamReaderWithRewinding> && reader), (override));
    };

    class SesameEncodedMock
        : public SesameEncoded
    {
    public:
        MOCK_METHOD(void, RequestSendMessage, (std::size_t size), (override));
        MOCK_METHOD(std::size_t, MaxSendMessageSize, (), (const, override));
        MOCK_METHOD(std::size_t, MessageSize, (std::size_t), (const, override));
    };

    class SesameEncodedObserverMock
        : public SesameEncodedObserver
    {
    public:
        using SesameEncodedObserver::SesameEncodedObserver;

        MOCK_METHOD(void, Initialized, (), (override));
        MOCK_METHOD(void, SendMessageStreamAvailable, (infra::SharedPtr<infra::StreamWriter> && writer), (override));
        MOCK_METHOD(void, MessageSent, (std::size_t encodedSize), (override));
        MOCK_METHOD(void, ReceivedMessage, (infra::SharedPtr<infra::StreamReaderWithRewinding> && reader), (override));
    };
}

#endif
