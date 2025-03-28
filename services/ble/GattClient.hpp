#ifndef SERVICES_GATT_CLIENT_HPP
#define SERVICES_GATT_CLIENT_HPP

#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
#include "infra/util/Observer.hpp"
#include "services/ble/Gatt.hpp"
#include <array>

namespace services
{
    class GattClientCharacteristicUpdate;

    class GattClientCharacteristicUpdateObserver
        : public infra::Observer<GattClientCharacteristicUpdateObserver, GattClientCharacteristicUpdate>
    {
    public:
        using infra::Observer<GattClientCharacteristicUpdateObserver, GattClientCharacteristicUpdate>::Observer;

        virtual void NotificationReceived(infra::ConstByteRange data) = 0;
        virtual void IndicationReceived(infra::ConstByteRange data, const infra::Function<void()>& onDone) = 0;
    };

    class GattClientCharacteristicUpdate
        : public infra::Subject<GattClientCharacteristicUpdateObserver>
    {};

    class GattClientCharacteristicOperations;

    class GattClientStackUpdateObserver
        : public infra::Observer<GattClientStackUpdateObserver, GattClientCharacteristicOperations>
    {
    public:
        using infra::Observer<GattClientStackUpdateObserver, GattClientCharacteristicOperations>::Observer;

        virtual void NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data) = 0;
        virtual void IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone) = 0;
    };

    class GattClientCharacteristicOperationsObserver
        : public infra::Observer<GattClientCharacteristicOperationsObserver, GattClientCharacteristicOperations>
    {
    public:
        using infra::Observer<GattClientCharacteristicOperationsObserver, GattClientCharacteristicOperations>::Observer;

        virtual AttAttribute::Handle CharacteristicValueHandle() const = 0;
        // virtual GattCharacteristic::PropertyFlags CharacteristicProperties() const = 0;
    };

    class GattClientCharacteristicOperations
        : public infra::Subject<GattClientCharacteristicOperationsObserver>
        , public infra::Subject<GattClientStackUpdateObserver>
    {
    public:
        using OperationOnDone = infra::Function<void(uint8_t)>;

        virtual void Read(const GattClientCharacteristicOperationsObserver& characteristic, infra::Function<void(const infra::ConstByteRange&)> onRead, OperationOnDone onDone) = 0;
        virtual void Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, OperationOnDone onDone) = 0;
        virtual void WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data) = 0;

        virtual void EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic, OperationOnDone onDone) = 0;
        virtual void DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic, OperationOnDone onDone) = 0;
        virtual void EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic, OperationOnDone onDone) = 0;
        virtual void DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic, OperationOnDone onDone) = 0;
    };

    class GattClientCharacteristic
        : public infra::IntrusiveForwardList<GattClientCharacteristic>::NodeType
        , public GattCharacteristic
        , public GattClientCharacteristicOperationsObserver
        , public GattClientCharacteristicUpdate
        , protected GattClientStackUpdateObserver
    {
    public:
        using OperationOnDone = GattClientCharacteristicOperations::OperationOnDone;

        GattClientCharacteristic(AttAttribute::Uuid type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties);
        GattClientCharacteristic(GattClientCharacteristicOperations& operations, AttAttribute::Uuid type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties);

        virtual void Read(infra::Function<void(const infra::ConstByteRange&)> onResponse, OperationOnDone onDone);
        virtual void Write(infra::ConstByteRange data, OperationOnDone onDone);
        virtual void WriteWithoutResponse(infra::ConstByteRange data);

        virtual void EnableNotification(OperationOnDone onDone);
        virtual void DisableNotification(OperationOnDone onDone);
        virtual void EnableIndication(OperationOnDone onDone);
        virtual void DisableIndication(OperationOnDone onDone);

        GattCharacteristic::PropertyFlags CharacteristicProperties() const;

        // Implementation of GattClientCharacteristicOperationsObserver
        AttAttribute::Handle CharacteristicValueHandle() const override;

        // Implementation of GattClientStackUpdateObserver
        void NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data) override;
        void IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone) override;

    private:
        infra::AutoResetFunction<void()> onIndicationDone;
        uint32_t observers;
    };

    class GattClientService
        : public GattService
        , public infra::IntrusiveForwardList<GattClientService>::NodeType
    {
    public:
        explicit GattClientService(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& endHandle);

        void AddCharacteristic(GattClientCharacteristic& characteristic);
        const infra::IntrusiveForwardList<GattClientCharacteristic>& Characteristics() const;

    private:
        infra::IntrusiveForwardList<GattClientCharacteristic> characteristics;
    };

    class GattClientDiscovery;

    class GattClientDiscoveryObserver
        : public infra::Observer<GattClientDiscoveryObserver, GattClientDiscovery>
    {
    public:
        using infra::Observer<GattClientDiscoveryObserver, GattClientDiscovery>::Observer;

        virtual void ServiceDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle) = 0;
        virtual void ServiceDiscoveryComplete() = 0;
        virtual void CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties) = 0;
        virtual void CharacteristicDiscoveryComplete() = 0;
        virtual void DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle) = 0;
        virtual void DescriptorDiscoveryComplete() = 0;
    };

    class GattClientDiscovery
        : public infra::Subject<GattClientDiscoveryObserver>
    {
    public:
        virtual void StartServiceDiscovery() = 0;
        virtual void StartCharacteristicDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) = 0;
        virtual void StartDescriptorDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) = 0;

        void StartCharacteristicDiscovery(const GattService& service)
        {
            StartCharacteristicDiscovery(service.Handle(), service.EndHandle());
        }

        void StartDescriptorDiscovery(const GattService& service)
        {
            StartDescriptorDiscovery(service.Handle(), service.EndHandle());
        }
    };

    class GattClientDiscoveryDecorator
        : public GattClientDiscoveryObserver
        , public GattClientDiscovery
    {
    public:
        using GattClientDiscoveryObserver::GattClientDiscoveryObserver;

        // Implementation of GattClientDiscoveryObserver
        void ServiceDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;
        void CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties) override;
        void DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle) override;

        void ServiceDiscoveryComplete() override;
        void CharacteristicDiscoveryComplete() override;
        void DescriptorDiscoveryComplete() override;

        // Implementation of GattClientDiscovery
        void StartServiceDiscovery() override;
        void StartCharacteristicDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;
        void StartDescriptorDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;
    };
}

#endif
