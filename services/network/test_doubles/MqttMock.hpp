#ifndef NETWORK_MQTT_MOCK_HPP
#define NETWORK_MQTT_MOCK_HPP

#include "infra/util/test_helper/BoundedStringMatcher.hpp"
#include "services/network/Mqtt.hpp"
#include "gmock/gmock.h"

namespace services
{
    class MqttClientObserverMock
        : public MqttClientObserver
    {
    public:
        MOCK_METHOD0(Attached, void());
        MOCK_METHOD0(PublishDone, void());
        MOCK_METHOD0(SubscribeDone, void());
        MOCK_METHOD2(ReceivedNotification, infra::SharedPtr<infra::StreamWriter>(infra::BoundedConstString topic, uint32_t payloadSize));
        MOCK_METHOD0(Detaching, void());
        MOCK_CONST_METHOD1(FillTopic, void(infra::StreamWriter& writer));
        MOCK_CONST_METHOD1(FillPayload, void(infra::StreamWriter& writer));
    };

    class MqttClientObserverFactoryMock
        : public MqttClientObserverFactory
    {
    public:
        MOCK_METHOD1(ConnectionEstablished, void(infra::AutoResetFunction<void(infra::SharedPtr<MqttClientObserver> client)>&& createdClientObserver));
        MOCK_METHOD1(ConnectionFailed, void(ConnectFailReason reason));
    };

    class MqttClientMock
        : public MqttClient
    {
    public:
        MOCK_METHOD0(Publish, void());
        MOCK_METHOD0(Subscribe, void());
        MOCK_METHOD0(NotificationDone, void());
        MOCK_METHOD0(Disconnect, void());
    };

    class MqttClientConnectorMock
        : public services::MqttClientConnector
    {
    public:
        MOCK_METHOD1(Connect, void(services::MqttClientObserverFactory& factory));
        MOCK_METHOD0(CancelConnect, void());
    };
}

#endif
