#include "services/network/MqttMultipleAccess.hpp"

namespace services
{
    void MqttMultipleAccessMaster::Publish()
    {
        MqttClientObserver::Subject().Publish();
    }

    void MqttMultipleAccessMaster::Subscribe()
    {
        MqttClientObserver::Subject().Subscribe();
    }

    void MqttMultipleAccessMaster::NotificationDone()
    {
        --notificationsSent;

        if (notificationsSent == 0)
            MqttClientObserver::Subject().NotificationDone();
    }

    void MqttMultipleAccessMaster::Connected()
    {
        for (auto& access : accesses)
            access.Connected();
    }

    void MqttMultipleAccessMaster::PublishDone()
    {
        GetObserver().PublishDone();
    }

    void MqttMultipleAccessMaster::SubscribeDone()
    {
        GetObserver().SubscribeDone();
    }

    void MqttMultipleAccessMaster::ReceivedNotification(infra::BoundedConstString topic, infra::BoundedConstString payload)
    {
        notificationsSent = 1;

        for (auto& access : accesses)
        {
            ++notificationsSent;
            access.ReceivedNotification(topic, payload);
        }

        NotificationDone();
    }

    void MqttMultipleAccessMaster::ClosingConnection()
    {
        for (auto& access : accesses)
            access.ClosingConnection();
    }

    void MqttMultipleAccessMaster::FillTopic(infra::StreamWriter& writer) const
    {
        GetObserver().FillTopic(writer);
    }

    void MqttMultipleAccessMaster::FillPayload(infra::StreamWriter& writer) const
    {
        GetObserver().FillPayload(writer);
    }

    MqttMultipleAccess::MqttMultipleAccess(MqttMultipleAccessMaster& master)
        : master(master)
        , claimer(master)
    {
        master.accesses.push_front(*this);
    }

    MqttMultipleAccess::~MqttMultipleAccess()
    {
        master.accesses.erase_slow(*this);
    }

    void MqttMultipleAccess::Publish()
    {
        claimer.Claim([this]()
        {
            Attach(master);
            master.Publish();
        });
    }

    void MqttMultipleAccess::Subscribe()
    {
        claimer.Claim([this]()
        {
            Attach(master);
            master.Subscribe();
        });
    }

    void MqttMultipleAccess::NotificationDone()
    {
        master.NotificationDone();
    }

    void MqttMultipleAccess::Connected()
    {
        GetObserver().Connected();
    }

    void MqttMultipleAccess::PublishDone()
    {
        Detach();
        claimer.Release();
        GetObserver().PublishDone();
    }

    void MqttMultipleAccess::SubscribeDone()
    {
        Detach();
        claimer.Release();
        GetObserver().SubscribeDone();
    }

    void MqttMultipleAccess::ReceivedNotification(infra::BoundedConstString topic, infra::BoundedConstString payload)
    {
        GetObserver().ReceivedNotification(topic, payload);
    }

    void MqttMultipleAccess::ClosingConnection()
    {
        GetObserver().ClosingConnection();
    }

    void MqttMultipleAccess::FillTopic(infra::StreamWriter& writer) const
    {
        GetObserver().FillTopic(writer);
    }

    void MqttMultipleAccess::FillPayload(infra::StreamWriter& writer) const
    {
        GetObserver().FillPayload(writer);
    }
}
