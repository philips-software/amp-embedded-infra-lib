#ifndef SERVICES_GATT_CLIENT_HPP
#define SERVICES_GATT_CLIENT_HPP

#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
#include "infra/util/Observer.hpp"
#include "services/ble/Gatt.hpp"

namespace services
{
    enum class OperationStatus : uint8_t
    {
        success,
        retry,
        error
    };

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

    class GattClientCharacteristicOperations
        : public infra::Subject<GattClientStackUpdateObserver>
    {
    public:
        virtual void Read(AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone) = 0;
        virtual void Write(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) = 0;
        virtual void WriteWithoutResponse(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) = 0;

        virtual void EnableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) = 0;
        virtual void DisableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) = 0;
        virtual void EnableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) = 0;
        virtual void DisableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) = 0;
    };

    class GattClientCharacteristic
        : public infra::IntrusiveForwardList<GattClientCharacteristic>::NodeType
        , public GattCharacteristic
        , public GattClientCharacteristicUpdate
        , protected GattClientStackUpdateObserver
    {
    public:
        GattClientCharacteristic(AttAttribute::Uuid type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties);
        GattClientCharacteristic(GattClientCharacteristicOperations& operations, AttAttribute::Uuid type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties);

        virtual void Read(const infra::Function<void(const infra::ConstByteRange&)>& onResponse, const infra::Function<void(OperationStatus)>& onDone);
        virtual void Write(infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone);
        virtual void WriteWithoutResponse(infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone);

        virtual void EnableNotification(const infra::Function<void(OperationStatus)>& onDone);
        virtual void DisableNotification(const infra::Function<void(OperationStatus)>& onDone);
        virtual void EnableIndication(const infra::Function<void(OperationStatus)>& onDone);
        virtual void DisableIndication(const infra::Function<void(OperationStatus)>& onDone);

        GattCharacteristic::PropertyFlags CharacteristicProperties() const;
        AttAttribute::Handle CharacteristicValueHandle() const;

        // Implementation of GattClientStackUpdateObserver
        void NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data) override;
        void IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone) override;

    private:
        GattClientCharacteristicOperations* operations{ nullptr };
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
        virtual void ServiceDiscoveryComplete(OperationStatus status) = 0;
        virtual void CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties) = 0;
        virtual void CharacteristicDiscoveryComplete(OperationStatus status) = 0;
        virtual void DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle) = 0;
        virtual void DescriptorDiscoveryComplete(OperationStatus status) = 0;
    };

    class GattClientDiscovery
        : public infra::Subject<GattClientDiscoveryObserver>
    {
    public:
        virtual void StartServiceDiscovery() = 0;
        virtual void StartCharacteristicDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) = 0;
        virtual void StartDescriptorDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) = 0;
    };

    class GattClientMtuExchange
    {
    public:
        virtual void MtuExchange(const infra::Function<void(OperationStatus)>& onDone) = 0;
    };

    class GattClient;

    class GattClientObserver
        : public infra::Observer<GattClientObserver, GattClient>
    {
    public:
        using infra::Observer<GattClientObserver, GattClient>::Observer;

        virtual void ServiceDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle) = 0;
        virtual void ServiceDiscoveryComplete(OperationStatus status) = 0;
        virtual void CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties) = 0;
        virtual void CharacteristicDiscoveryComplete(OperationStatus status) = 0;
        virtual void DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle) = 0;
        virtual void DescriptorDiscoveryComplete(OperationStatus status) = 0;

        virtual void NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data) = 0;
        virtual void IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone) = 0;
    };

    class GattClient
        : public infra::Subject<GattClientObserver>
    {
    public:
        virtual void StartServiceDiscovery() = 0;
        virtual void StartCharacteristicDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) = 0;
        virtual void StartDescriptorDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) = 0;

        virtual void Read(AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone) = 0;
        virtual void Write(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) = 0;
        virtual void WriteWithoutResponse(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) = 0;
        virtual void EnableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) = 0;
        virtual void DisableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) = 0;
        virtual void EnableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) = 0;
        virtual void DisableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) = 0;

        virtual void MtuExchange(const infra::Function<void(OperationStatus)>& onDone) = 0;
    };
}

namespace infra
{
    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::OperationStatus& status);
}

#endif
