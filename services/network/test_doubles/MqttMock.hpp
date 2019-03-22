#ifndef NETWORK_MQTT_MOCK_HPP
#define NETWORK_MQTT_MOCK_HPP

#include "gmock/gmock.h"
#include "infra/util/test_helper/BoundedStringMatcher.hpp"
#include "services/network/Mqtt.hpp"

namespace services
{
    class MqttClientObserverMock
        : public MqttClientObserver
    {
    public:
        MOCK_METHOD0(Connected, void());
        MOCK_METHOD0(PublishDone, void());
        MOCK_METHOD0(ClosingConnection, void());
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
        MOCK_METHOD2(Publish, void(infra::BoundedConstString topic, infra::BoundedConstString payload));
    };
}

#endif
