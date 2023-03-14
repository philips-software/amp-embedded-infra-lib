#ifndef SERVICES_GATT_CLIENT_HPP
#define SERVICES_GATT_CLIENT_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/EnumCast.hpp"
#include "infra/util/Function.hpp"
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
        virtual void Read(infra::Function<void(infra::ConstByteRange&)> onDone) = 0;
        virtual void Write(infra::ConstByteRange data, infra::Function<void()> onDone) = 0;
        virtual void WriteWithoutResponse(infra::ConstByteRange data) = 0;
    };

    class GattClientIndicationNotification;

    class GattClientNotificationObserver
        : public infra::Observer<GattClientNotificationObserver, GattClientIndicationNotification>
    {
    public:
        using infra::Observer<GattClientNotificationObserver, GattClientIndicationNotification>::Observer;

        virtual void ConfigurationCompleted() = 0;
        virtual void NotificationReceived(const AttAttribute::Handle& handle, infra::ConstByteRange data) = 0;
    };

    class GattClientIndicationNotification
        : public infra::Subject<GattClientNotificationObserver>
    {
    public:
        virtual void EnableNotification(infra::Function<void()> onDone) = 0;
        virtual void DisableNotification(infra::Function<void()> onDone) = 0;

        virtual void EnableIndication(infra::Function<void()> onDone) = 0;
        virtual void DisableIndication(infra::Function<void()> onDone) = 0;
    };

    class GattClientCharacteristicOperations;

    class GattClientCharacteristicOperationsObserver
        : public infra::Observer<GattClientCharacteristicOperationsObserver, GattClientCharacteristicOperations>
    {
    public:
        using infra::Observer<GattClientCharacteristicOperationsObserver, GattClientCharacteristicOperations>::Observer;

        virtual AttAttribute::Handle CharacteristicHandle() const = 0;
    };

    class GattClientCharacteristicOperations
        : public infra::Subject<GattClientCharacteristicOperationsObserver>
    {
    public:
        virtual bool Read(const GattClientCharacteristicOperationsObserver& characteristic) const = 0;
        virtual bool Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data) const = 0;
        virtual void WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data) const = 0;

        virtual bool EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic) const = 0;
        virtual bool DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic) const = 0;

        virtual bool EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic) const = 0;
        virtual bool DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic) const = 0;
    };

    /* class GattClientDescriptor
    {
    public:
        explicit GattClientDescriptor(AttAttribute::Uuid type, AttAttribute::Handle handle);

        virtual void EnableNotification() = 0;
        virtual void DisableNotification() = 0;

        virtual void EnableIndication() = 0;
        virtual void DisableIndication() = 0;

    private:
        struct ClientCharacteristicConfiguration
        {
            const uint16_t attributeType = 0x2902;

            enum class CharacteristicValue : uint16_t
            {
                enableNotification = 0x0001,
                enableIndication = 0x0002,
            };
        };

    private:
        AttAttribute::Uuid type;
        AttAttribute::Handle handle;
    };*/

    class GattClientCharacteristic
        : public infra::IntrusiveForwardList<GattClientCharacteristic>::NodeType
        , public GattCharacteristic
        , public GattClientCharacteristicOperationsObserver
        , public GattClientDataOperation
        , public GattClientIndicationNotification
    {
    public:
        GattClientCharacteristic() = default;
        GattClientCharacteristic(GattClientCharacteristic& other) = delete;
        GattClientCharacteristic& operator=(const GattClientCharacteristic& other) = delete;
        virtual ~GattClientCharacteristic() = default;

        /* void AddDescriptor(GattClientCharacteristic& characteristic);
        const infra::IntrusiveForwardList<GattClientCharacteristic>& Descriptors() const;*/

    private:
        /* infra::IntrusiveForwardList<GattClientDescriptor> descriptors;*/
    };

    class GattClientService
        : public GattService
        , public infra::IntrusiveForwardList<GattClientService>::NodeType
    {
    public:
        explicit GattClientService(const AttAttribute::Uuid& type);
        GattClientService(GattClientService& other) = delete;
        GattClientService& operator=(const GattClientService& other) = delete;
        virtual ~GattClientService() = default;

        void AddCharacteristic(GattClientCharacteristic& characteristic);
        // infra::IntrusiveForwardList<GattClientCharacteristic>& Characteristics();
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

        virtual void ServiceDiscovered(const GattClientService& service) = 0;
        virtual void CharacteristicDiscovered(const GattCharacteristic& characteristic) = 0;
        virtual void DescriptorDiscovered(const GattDescriptor& descriptor) = 0;

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
        virtual void StartDescriptorDiscovery(const GattCharacteristic& service) = 0;
    };
}

#endif
