#ifndef SERVICES_GATT_SERVER_HPP
#define SERVICES_GATT_SERVER_HPP

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
    class GattCharacteristicUpdate;

    class GattCharacteristicObserver
        : public infra::Observer<GattCharacteristicObserver, GattCharacteristicUpdate>
    {
    public:
        using infra::Observer<GattCharacteristicObserver, GattCharacteristicUpdate>::Observer;

        virtual void DataReceived(infra::ConstByteRange data) = 0;
    };

    class GattCharacteristicUpdate
        : public infra::Subject<GattCharacteristicObserver>
    {
    public:
        virtual void Update(infra::ConstByteRange data, infra::Function<void()> onDone) = 0;
    };

    class GattCharacteristicClientOperations;

    class GattCharacteristicClientOperationsObserver
        : public infra::Observer<GattCharacteristicClientOperationsObserver, GattCharacteristicClientOperations>
    {
    public:
        using infra::Observer<GattCharacteristicClientOperationsObserver, GattCharacteristicClientOperations>::Observer;

        virtual AttAttribute::Handle ServiceHandle() const = 0;
        virtual AttAttribute::Handle CharacteristicHandle() const = 0;
    };

    class GattCharacteristicClientOperations
        : public infra::Subject<GattCharacteristicClientOperationsObserver>
    {
    public:
        enum class UpdateStatus : uint8_t
        {
            success,
            retry,
            error
        };

        // Update 'characteristic' with 'data' towards the
        // BLE stack and, depending on the configuration of
        // that 'characteristic', send a notification or indication.
        // Returns success, or retry in transient failure or error
        // on unrecoverable failure (i.e. BLE stack indicates an issue
        // with updating or sending data).
        virtual UpdateStatus Update(const GattCharacteristicClientOperationsObserver& characteristic, infra::ConstByteRange data) const = 0;
    };

    class GattServerCharacteristic
        : public infra::IntrusiveForwardList<GattServerCharacteristic>::NodeType
        , public GattCharacteristic
        , public GattCharacteristicClientOperationsObserver
        , public GattCharacteristicUpdate
    {
    public:
        // Description in Bluetooth Core Specification
        // Volume 3, Part F, section 3.2.5
        enum class PermissionFlags : uint8_t
        {
            none = 0x00u,
            authenticatedRead = 0x01u,
            authorizedRead = 0x02u,
            encryptedRead = 0x04u,
            authenticatedWrite = 0x08u,
            authorizedWrite = 0x10u,
            encryptedWrite = 0x20u
        };

        GattServerCharacteristic() = default;
        GattServerCharacteristic(GattServerCharacteristic& other) = delete;
        GattServerCharacteristic& operator=(const GattServerCharacteristic& other) = delete;
        virtual ~GattServerCharacteristic() = default;

        virtual PermissionFlags Permissions() const = 0;
        virtual uint16_t ValueLength() const = 0;
        virtual uint8_t GetAttributeCount() const = 0;
    };

    class GattServerService
        : public GattService
        , public infra::IntrusiveForwardList<GattServerService>::NodeType
    {
    public:
        explicit GattServerService(const AttAttribute::Uuid& type);
        GattServerService(GattServerService& other) = delete;
        GattServerService& operator=(const GattServerService& other) = delete;
        virtual ~GattServerService() = default;

        void AddCharacteristic(GattServerCharacteristic& characteristic);
        infra::IntrusiveForwardList<GattServerCharacteristic>& Characteristics();
        const infra::IntrusiveForwardList<GattServerCharacteristic>& Characteristics() const;

        uint8_t GetAttributeCount() const;

    private:
        infra::IntrusiveForwardList<GattServerCharacteristic> characteristics;
    };

    class GattServer
    {
    public:
        virtual void AddService(GattServerService& service) = 0;
    };

    inline GattServerCharacteristic::PermissionFlags operator|(GattServerCharacteristic::PermissionFlags lhs, GattServerCharacteristic::PermissionFlags rhs)
    {
        return static_cast<GattServerCharacteristic::PermissionFlags>(infra::enum_cast(lhs) | infra::enum_cast(rhs));
    }
}

#endif
