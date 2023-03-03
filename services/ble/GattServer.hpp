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
    class GattServerCharacteristic
        : public infra::IntrusiveForwardList<GattServerCharacteristic>::NodeType
        , public GattCharacteristic
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

        void AddCharacteristic(GattServerCharacteristic& characteristic) { characteristics.push_front(characteristic); }
        infra::IntrusiveForwardList<GattServerCharacteristic>& Characteristics() { return characteristics; }
        const infra::IntrusiveForwardList<GattServerCharacteristic>& Characteristics() const { return characteristics; }

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
