#ifndef SERVICES_GATT_HPP
#define SERVICES_GATT_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/EnumCast.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/Variant.hpp"
#include <array>

namespace services
{
    struct GattAttribute
    {
        using Uuid16 = uint16_t;
        using Uuid128 = std::array<uint8_t, 16>;
        using Uuid = infra::Variant<Uuid16, Uuid128>;

        using Handle = uint16_t;

        const Uuid type;
        Handle handle;
    };

    class GattCharacteristic;

    class GattCharacteristicObserver
        : public infra::Observer<GattCharacteristicObserver, GattCharacteristic>
    {
    public:
        using infra::Observer<GattCharacteristicObserver, GattCharacteristic>::Observer;

        virtual void DataReceived(infra::ConstByteRange data) = 0;
    };

    class GattCharacteristicClientOperations;

    class GattCharacteristicClientOperationsObserver
        : public infra::Observer<GattCharacteristicClientOperationsObserver, GattCharacteristicClientOperations>
    {
    public:
        using infra::Observer<GattCharacteristicClientOperationsObserver, GattCharacteristicClientOperations>::Observer;

        virtual GattAttribute::Handle ServiceHandle() const = 0;
        virtual GattAttribute::Handle CharacteristicHandle() const = 0;
    };

    class GattCharacteristicClientOperations
        : public infra::Subject<GattCharacteristicClientOperationsObserver>
    {
    public:
        // Update 'characteristic' with 'data' towards the
        // BLE stack and, depending on the configuration of
        // that 'characteristic', send a notification or indication.
        // Returns true on success, or false on failure (i.e. BLE
        // stack indicates an issue with updating or sending data).
        virtual bool Update(const GattCharacteristicClientOperationsObserver& characteristic, infra::ConstByteRange data) const = 0;
    };

    class GattCharacteristic
        : public GattCharacteristicClientOperationsObserver
        , public infra::Subject<GattCharacteristicObserver>
        , public infra::IntrusiveForwardList<GattCharacteristic>::NodeType
    {
    public:
        // Values taken from Bluetooth Core Specification
        // Volume 3, Part G, section 3.3.1.1
        enum class Properties : uint8_t
        {
            none = 0x00,
            broadcast = 0x01,
            read = 0x02,
            writeWithoutResponse = 0x04,
            write = 0x08,
            notify = 0x10,
            indicate = 0x20,
            signedWrite = 0x40,
            extended = 0x80
        };

        // Description in Bluetooth Core Specification
        // Volume 3, Part F, section 3.2.5
        enum class Permissions : uint8_t
        {
            none = 0x00,
            authenticatedRead = 0x01,
            authorizedRead = 0x02,
            encryptedRead = 0x04,
            authenticatedWrite = 0x08,
            authorizedWrite = 0x10,
            encryptedWrite = 0x20
        };

    public:
        GattCharacteristic() = default;
        GattCharacteristic(GattCharacteristic& other) = delete;
        GattCharacteristic& operator=(const GattCharacteristic& other) = delete;

        virtual GattAttribute::Uuid Type() const = 0;
        virtual Properties CharacteristicProperties() const = 0;
        virtual Permissions CharacteristicPermissions() const = 0;

        virtual GattAttribute::Handle Handle() const = 0;
        virtual GattAttribute::Handle& Handle() = 0;

        virtual uint16_t ValueLength() const = 0;

        virtual void Update(infra::ConstByteRange data, infra::Function<void()> onDone) = 0;
    };

    inline GattCharacteristic::Properties operator|(GattCharacteristic::Properties lhs, GattCharacteristic::Properties rhs)
    {
        return static_cast<GattCharacteristic::Properties>(infra::enum_cast(lhs) | infra::enum_cast(rhs));
    }

    inline GattCharacteristic::Permissions operator|(GattCharacteristic::Permissions lhs, GattCharacteristic::Permissions rhs)
    {
        return static_cast<GattCharacteristic::Permissions>(infra::enum_cast(lhs) | infra::enum_cast(rhs));
    }

    class GattService
    {
    public:
        GattService(const GattAttribute::Uuid& type);
        GattService(GattService& other) = delete;
        GattService& operator=(const GattService& other) = delete;

        void AddCharacteristic(const GattCharacteristic& characteristic);
        infra::IntrusiveForwardList<GattCharacteristic>& Characteristics();

        GattAttribute::Uuid Type() const;

        GattAttribute::Handle Handle() const;
        GattAttribute::Handle& Handle();

    private:
        GattAttribute attribute;
        infra::IntrusiveForwardList<GattCharacteristic> characteristics;
    };

    class GattServer
    {
    public:
        virtual void AddService(GattService& service) = 0;
    };
}

#endif
