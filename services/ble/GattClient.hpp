#ifndef SERVICES_GATT_CLIENT_HPP
#define SERVICES_GATT_CLIENT_HPP

#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/EnumCast.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/Variant.hpp"
#include "services/ble/Gatt.hpp"
#include <array>

namespace services
{
    class GattClientDataOperation;

    class GattClientDataOperationObserver
        : public infra::Observer<GattClientDataOperationObserver, GattClientDataOperation>
    {
    public:
        using infra::Observer<GattClientDataOperationObserver, GattClientDataOperation>::Observer;

        virtual void ReadCompleted(infra::ConstByteRange data) = 0;
        virtual void WriteCompleted() = 0;
    };

    class GattClientDataOperation
        : public infra::Subject<GattClientDataOperationObserver>
    {
    public:
        virtual void Read(infra::Function<void(const infra::ConstByteRange&)> onResponse) = 0;
        virtual void Write(infra::ConstByteRange data, infra::Function<void()> onDone) = 0;
        virtual void WriteWithoutResponse(infra::ConstByteRange data) = 0;
    };

    class GattClientUpdate;

    class GattClientUpdateObserver
        : public infra::Observer<GattClientUpdateObserver, GattClientUpdate>
    {
    public:
        using infra::Observer<GattClientUpdateObserver, GattClientUpdate>::Observer;

        virtual void ConfigurationCompleted() = 0;
        virtual void UpdateReceived(const AttAttribute::Handle& handle, infra::ConstByteRange data) = 0;
    };

    class GattClientUpdate
        : public infra::Subject<GattClientUpdateObserver>
    {
    public:
        virtual void EnableNotification(infra::Function<void()> onDone) = 0;
        virtual void DisableNotification(infra::Function<void()> onDone) = 0;

        virtual void EnableIndication(infra::Function<void()> onDone) = 0;
        virtual void DisableIndication(infra::Function<void()> onDone) = 0;

        virtual void Update(infra::Function<void(const infra::ConstByteRange&)> onUpdate) = 0;
    };

    class GattClientCharacteristicOperations;

    class GattClientCharacteristicOperationsObserver
        : public infra::Observer<GattClientCharacteristicOperationsObserver, GattClientCharacteristicOperations>
    {
    public:
        using infra::Observer<GattClientCharacteristicOperationsObserver, GattClientCharacteristicOperations>::Observer;

        virtual const AttAttribute::Handle& CharacteristicHandle() const = 0;
        virtual const GattCharacteristic::PropertyFlags& CharacteristicProperties() const = 0;
    };

    class GattClientCharacteristicOperations
        : public infra::Subject<GattClientCharacteristicOperationsObserver>
    {
    public:
        virtual void Read(const GattClientCharacteristicOperationsObserver& characteristic, infra::Function<void(const infra::ConstByteRange&)> onDone) const = 0;
        virtual void Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, infra::Function<void()> onDone) const = 0;
        virtual void WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data) const = 0;

        virtual void EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic, infra::Function<void()> onDone) const = 0;
        virtual void DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic, infra::Function<void()> onDone) const = 0;

        virtual void EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic, infra::Function<void()> onDone) const = 0;
        virtual void DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic, infra::Function<void()> onDone) const = 0;
    };

    class GattClientCharacteristic;

    class GattClientDescriptor
        : public infra::IntrusiveForwardList<GattClientDescriptor>::NodeType
        , public GattDescriptor
    {
    public:
        explicit GattClientDescriptor(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle);
        GattClientDescriptor(GattClientDescriptor& other) = delete;
        GattClientDescriptor& operator=(const GattClientDescriptor& other) = delete;
        GattClientDescriptor(GattClientDescriptor&& other) = default;
        GattClientDescriptor& operator=(GattClientDescriptor&& other) = default;
        virtual ~GattClientDescriptor() = default;
    };

    class GattClientService;

    class GattClientCharacteristic
        : public infra::IntrusiveForwardList<GattClientCharacteristic>::NodeType
        , public GattCharacteristic
    {
    public:
        explicit GattClientCharacteristic(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& valueHandle, const GattCharacteristic::PropertyFlags& properties);
        GattClientCharacteristic(GattClientCharacteristic& other) = delete;
        GattClientCharacteristic& operator=(const GattClientCharacteristic& other) = delete;
        GattClientCharacteristic(GattClientCharacteristic &&) = default;
        GattClientCharacteristic &operator=(GattClientCharacteristic &&) = default;
        virtual ~GattClientCharacteristic() = default;

        void AddDescriptor(GattClientDescriptor& descriptor);
        const infra::IntrusiveForwardList<GattClientDescriptor>& Descriptors() const;

    protected:
        infra::IntrusiveForwardList<GattClientDescriptor> descriptors;
    };

    class GattClientService
        : public GattService
        , public infra::IntrusiveForwardList<GattClientService>::NodeType
    {
    public:
        explicit GattClientService(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& endHandle);
        GattClientService(GattClientService& other) = delete;
        GattClientService& operator=(const GattClientService& other) = delete;
        GattClientService(GattClientService&& other) = default;
        GattClientService& operator=(GattClientService&& other) = default;
        virtual ~GattClientService() = default;

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

        virtual void ServiceDiscovered(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& endHandle) = 0;
        virtual void CharacteristicDiscovered(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& valueHandle, const GattCharacteristic::PropertyFlags& properties) = 0;
        virtual void DescriptorDiscovered(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle) = 0;

        virtual void ServiceDiscoveryComplete() = 0;
        virtual void CharacteristicDiscoveryComplete() = 0;
        virtual void DescriptorDiscoveryComplete() = 0;
    };

    class GattClientDiscovery
        : public infra::Subject<GattClientDiscoveryObserver>
    {
    public:
        virtual void StartServiceDiscovery() = 0;
        virtual void StartCharacteristicDiscovery(const GattService& service) = 0;
        virtual void StartDescriptorDiscovery(const GattService& service) = 0;
    };

    class GattClientDiscoveryDecorator
        : public GattClientDiscoveryObserver
        , public GattClientDiscovery
    {
    public:
        using GattClientDiscoveryObserver::GattClientDiscoveryObserver;

        // Implementation of GattClientDiscoveryObserver
        virtual void ServiceDiscovered(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& endHandle) override;
        virtual void CharacteristicDiscovered(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& valueHandle, const GattCharacteristic::PropertyFlags& properties) override;
        virtual void DescriptorDiscovered(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle) override;

        virtual void ServiceDiscoveryComplete() override;
        virtual void CharacteristicDiscoveryComplete() override;
        virtual void DescriptorDiscoveryComplete() override;

        // Implementation of GattClientDiscovery
        virtual void StartServiceDiscovery() override;
        virtual void StartCharacteristicDiscovery(const GattService& service) override;
        virtual void StartDescriptorDiscovery(const GattService& service) override;
    };
}

#endif
