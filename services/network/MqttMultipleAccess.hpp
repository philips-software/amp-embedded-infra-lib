#ifndef SERVICES_MQTT_MULTIPLE_ACCESS_HPP
#define SERVICES_MQTT_MULTIPLE_ACCESS_HPP

#include "services/network/Mqtt.hpp"
#include "infra/event/ClaimableResource.hpp"
#include "infra/util/IntrusiveForwardList.hpp"

namespace services
{
    class MqttMultipleAccess;

    class MqttMultipleAccessMaster                                                                             //TICS !OOP#013
        : public MqttClient
        , public MqttClientObserver
        , public infra::ClaimableResource
    {
    public:
        // Implementation of MqttClient
        virtual void Publish() override;
        virtual void Subscribe() override;
        virtual void NotificationDone() override;

        // Implementation of MqttClientObserver
        virtual void Connected() override;
        virtual void PublishDone() override;
        virtual void SubscribeDone() override;
        virtual infra::SharedPtr<infra::StreamWriter> ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize) override;
        virtual void ClosingConnection() override;
        virtual void FillTopic(infra::StreamWriter& writer) const override;
        virtual void FillPayload(infra::StreamWriter& writer) const override;

    private:
        infra::IntrusiveForwardList<MqttMultipleAccess> accesses;

        friend class MqttMultipleAccess;
    };

    class MqttMultipleAccess
        : public MqttClient
        , public MqttClientObserver
        , public infra::IntrusiveForwardList<MqttMultipleAccess>::NodeType
    {
    public:
        explicit MqttMultipleAccess(MqttMultipleAccessMaster& master);
        ~MqttMultipleAccess();

        // Implementation of MqttClient
        virtual void Publish() override;
        virtual void Subscribe() override;
        virtual void NotificationDone() override;

        // Implementation of MqttClientObserver
        virtual void Connected() override;
        virtual void PublishDone() override;
        virtual void SubscribeDone() override;
        virtual infra::SharedPtr<infra::StreamWriter> ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize) override;
        virtual void ClosingConnection() override;
        virtual void FillTopic(infra::StreamWriter& writer) const override;
        virtual void FillPayload(infra::StreamWriter& writer) const override;

    private:
        MqttMultipleAccessMaster& master;
        infra::ClaimableResource::Claimer claimer;
    };
}

#endif
