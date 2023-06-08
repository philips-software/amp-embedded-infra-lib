#ifndef SERVICES_MQTT_MULTIPLE_ACCESS_HPP
#define SERVICES_MQTT_MULTIPLE_ACCESS_HPP

#include "infra/event/ClaimableResource.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
#include "services/network/Mqtt.hpp"

namespace services
{
    class MqttMultipleAccess;

    class MqttMultipleAccessMaster
        : public MqttClientObserver
        , public infra::ClaimableResource
    {
    public:
        void Register(MqttMultipleAccess& access);
        void Unregister(MqttMultipleAccess& access);
        void Publish(MqttMultipleAccess& access);
        void Subscribe(MqttMultipleAccess& access);
        void NotificationDone();
        void Disconnect();
        void ReleaseActive();

        // Implementation of MqttClientObserver
        void Attached() override;
        void Detaching() override;
        void PublishDone() override;
        void SubscribeDone() override;
        infra::SharedPtr<infra::StreamWriter> ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize) override;
        void FillTopic(infra::StreamWriter& writer) const override;
        void FillPayload(infra::StreamWriter& writer) const override;

    private:
        infra::IntrusiveForwardList<MqttMultipleAccess> accesses;
        MqttMultipleAccess* active = nullptr;
    };

    class MqttMultipleAccess
        : public MqttClient
        , public infra::IntrusiveForwardList<MqttMultipleAccess>::NodeType
    {
    public:
        explicit MqttMultipleAccess(MqttMultipleAccessMaster& master, MqttClientObserver& observer);
        ~MqttMultipleAccess();

        // Implementation of MqttClient
        void Publish() override;
        void Subscribe() override;
        void NotificationDone() override;
        void Disconnect() override;

        void Attached();
        void Detaching();
        void PublishDone();
        void SubscribeDone();
        infra::SharedPtr<infra::StreamWriter> ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize);
        void FillTopic(infra::StreamWriter& writer) const;
        void FillPayload(infra::StreamWriter& writer) const;

    private:
        MqttMultipleAccessMaster& master;
        infra::ClaimableResource::Claimer claimer;
        MqttClientObserver& observer;
    };
}

#endif
