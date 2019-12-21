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

    infra::SharedPtr<infra::StreamWriter> MqttMultipleAccessMaster::ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize)
    {
        for (auto& access : accesses)
        {
            auto result = access.ReceivedNotification(topic, payloadSize);
            if (result != nullptr)
                return result;
        }

        std::abort();
    }

    void MqttMultipleAccessMaster::Detaching()
    {
        for (auto& access : accesses)
            access.Detaching();
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

    infra::SharedPtr<infra::StreamWriter> MqttMultipleAccess::ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize)
    {
        return GetObserver().ReceivedNotification(topic, payloadSize);
    }

    void MqttMultipleAccess::Detaching()
    {
        GetObserver().Detaching();
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
