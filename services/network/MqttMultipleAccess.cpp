#include "services/network/MqttMultipleAccess.hpp"

namespace services
{
    void MqttMultipleAccessMaster::Register(MqttMultipleAccess& access)
    {
        accesses.push_front(access);
    }

    void MqttMultipleAccessMaster::Unregister(MqttMultipleAccess& access)
    {
        accesses.erase_slow(access);
    }

    void MqttMultipleAccessMaster::Publish(MqttMultipleAccess& access)
    {
        assert(active == nullptr);
        active = &access;
        MqttClientObserver::Subject().Publish();
    }

    void MqttMultipleAccessMaster::Subscribe(MqttMultipleAccess& access)
    {
        assert(active == nullptr);
        active = &access;
        MqttClientObserver::Subject().Subscribe();
    }

    void MqttMultipleAccessMaster::NotificationDone()
    {
        MqttClientObserver::Subject().NotificationDone();
    }

    void MqttMultipleAccessMaster::ReleaseActive()
    {
        assert(active != nullptr);
        active = nullptr;
    }

    void MqttMultipleAccessMaster::Attached()
    {
        for (auto& access : accesses)
            access.Attached();
    }

    void MqttMultipleAccessMaster::Detaching()
    {
        for (auto& access : accesses)
            access.Detaching();
    }

    void MqttMultipleAccessMaster::PublishDone()
    {
        active->PublishDone();
    }

    void MqttMultipleAccessMaster::SubscribeDone()
    {
        active->SubscribeDone();
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

    void MqttMultipleAccessMaster::FillTopic(infra::StreamWriter& writer) const
    {
        active->FillTopic(writer);
    }

    void MqttMultipleAccessMaster::FillPayload(infra::StreamWriter& writer) const
    {
        active->FillPayload(writer);
    }

    MqttMultipleAccess::MqttMultipleAccess(MqttMultipleAccessMaster& master, MqttClientObserver& observer)
        : master(master)
        , claimer(master)
        , observer(observer)
    {
        master.Register(*this);
        if (master.IsAttached())
            Attached();
    }

    MqttMultipleAccess::~MqttMultipleAccess()
    {
        if (master.IsAttached())
            Detaching();
        master.Unregister(*this);
    }

    void MqttMultipleAccess::Publish()
    {
        claimer.Claim([this]()
        {
            master.Publish(*this);
        });
    }

    void MqttMultipleAccess::Subscribe()
    {
        claimer.Claim([this]()
        {
            master.Subscribe(*this);
        });
    }

    void MqttMultipleAccess::NotificationDone()
    {
        master.NotificationDone();
    }

    void MqttMultipleAccess::Attached()
    {
        Attach(infra::UnOwnedSharedPtr(observer));
    }

    void MqttMultipleAccess::Detaching()
    {
        Detach();
    }

    void MqttMultipleAccess::PublishDone()
    {
        master.ReleaseActive();
        claimer.Release();
        Observer().PublishDone();
    }

    void MqttMultipleAccess::SubscribeDone()
    {
        master.ReleaseActive();
        claimer.Release();
        Observer().SubscribeDone();
    }

    infra::SharedPtr<infra::StreamWriter> MqttMultipleAccess::ReceivedNotification(infra::BoundedConstString topic, uint32_t payloadSize)
    {
        return Observer().ReceivedNotification(topic, payloadSize);
    }

    void MqttMultipleAccess::FillTopic(infra::StreamWriter& writer) const
    {
        Observer().FillTopic(writer);
    }

    void MqttMultipleAccess::FillPayload(infra::StreamWriter& writer) const
    {
        Observer().FillPayload(writer);
    }
}
