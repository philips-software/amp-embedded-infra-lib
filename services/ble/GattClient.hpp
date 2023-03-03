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
    class GattClientDescriptor
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
    };

    class GattClientCharacteristic
        : public infra::IntrusiveForwardList<GattClientCharacteristic>::NodeType
        , public GattCharacteristic
    {
    public:
        virtual void Read() = 0;
        virtual void Write(infra::ConstByteRange data) = 0;
        virtual void WriteWithoutResponse(infra::ConstByteRange data) = 0;
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

        void AddCharacteristic(GattClientCharacteristic& characteristic) { characteristics.push_front(characteristic); }
        infra::IntrusiveForwardList<GattClientCharacteristic>& Characteristics() { return characteristics; }
        const infra::IntrusiveForwardList<GattClientCharacteristic>& Characteristics() const { return characteristics; }

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

    class GattClientDataOperation;

    class GattClientDataOperationObserver
        : public infra::Observer<GattClientDataOperationObserver, GattClientDataOperation>
    {
    public:
        using infra::Observer<GattClientDataOperationObserver, GattClientDataOperation>::Observer;

        enum class Result
        {
            Success,
            InsufficientAuthentication,
            UnknownError,
        };

        virtual void ReadCompleted(Result result, infra::ConstByteRange data) = 0;
        virtual void WriteCompleted(Result result) = 0;
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

    class GattClientDiscovery
        : public infra::Subject<GattClientDiscoveryObserver>
    {
    public:
        virtual void StartServiceDiscovery() = 0;
        virtual void StartCharacteristicDiscovery(const GattService& service) = 0;
        virtual void StartDescriptorDiscovery(const GattCharacteristic& service) = 0;
    };

    class GattClientIndicationNotification
        : public infra::Subject<GattClientNotificationObserver>
    {
    public:
        virtual void EnableNotification(const services::GattCharacteristic& characteristic) = 0;
        virtual void DisableNotification(const services::GattCharacteristic& characteristic) = 0;

        virtual void EnableIndication(const services::GattCharacteristic& characteristic) = 0;
        virtual void DisableIndication(const services::GattCharacteristic& characteristic) = 0;
    };

    class GattClientDataOperation
        : public infra::Subject<GattClientDataOperationObserver>
    {
    public:
        virtual void Read(const GattCharacteristic& characteristic) = 0;
        virtual void Write(const GattCharacteristic& characteristic, infra::ConstByteRange data) = 0;
        virtual void WriteWithoutResponse(const GattCharacteristic& characteristic, infra::ConstByteRange data) = 0;
    };
}

#endif
