#ifndef SERVICES_GATT_HPP
#define SERVICES_GATT_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Endian.hpp"
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
        using Uuid128 = infra::BigEndian<std::array<uint8_t, 16>>;
        using Uuid = infra::Variant<Uuid16, Uuid128>;

        using Handle = uint16_t;

        const Uuid type;
        Handle handle;
    };

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

        virtual GattAttribute::Handle ServiceHandle() const = 0;
        virtual GattAttribute::Handle CharacteristicHandle() const = 0;
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

    class GattCharacteristic
        : public GattCharacteristicClientOperationsObserver
        , public GattCharacteristicUpdate
        , public infra::IntrusiveForwardList<GattCharacteristic>::NodeType
    {
    public:
        // Values taken from Bluetooth Core Specification
        // Volume 3, Part G, section 3.3.1.1
        enum class PropertyFlags : uint8_t
        {
            none = 0x00u,
            broadcast = 0x01u,
            read = 0x02u,
            writeWithoutResponse = 0x04u,
            write = 0x08u,
            notify = 0x10u,
            indicate = 0x20u,
            signedWrite = 0x40u,
            extended = 0x80u
        };

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

    public:
        GattCharacteristic() = default;
        GattCharacteristic(GattCharacteristic& other) = delete;
        GattCharacteristic& operator=(const GattCharacteristic& other) = delete;
        virtual ~GattCharacteristic() = default;

        virtual PropertyFlags Properties() const = 0;
        virtual PermissionFlags Permissions() const = 0;
        virtual uint8_t GetAttributeCount() const = 0;

        virtual GattAttribute::Uuid Type() const = 0;
        virtual GattAttribute::Handle Handle() const = 0;
        virtual GattAttribute::Handle& Handle() = 0;

        virtual uint16_t ValueLength() const = 0;
    };

    class GattService
        : public infra::IntrusiveForwardList<GattService>::NodeType
    {
    public:
        explicit GattService(const GattAttribute::Uuid& type);
        GattService(GattService& other) = delete;
        GattService& operator=(const GattService& other) = delete;
        virtual ~GattService() = default;

        void AddCharacteristic(const GattCharacteristic& characteristic);
        infra::IntrusiveForwardList<GattCharacteristic>& Characteristics();
        const infra::IntrusiveForwardList<GattCharacteristic>& Characteristics() const;

        GattAttribute::Uuid Type() const;
        GattAttribute::Handle Handle() const;
        GattAttribute::Handle& Handle();

        uint8_t GetAttributeCount() const;

    private:
        GattAttribute attribute;
        infra::IntrusiveForwardList<GattCharacteristic> characteristics;
    };

    class GattServer
    {
    public:
        virtual void AddService(GattService& service) = 0;
    };

    class AttMtuExchange;

    class AttMtuExchangeObserver
        : public infra::Observer<AttMtuExchangeObserver, AttMtuExchange>
    {
    public:
        using infra::Observer<AttMtuExchangeObserver, AttMtuExchange>::Observer;

        virtual void ExchangedMaxAttMtuSize() = 0;
    };

    class AttMtuExchange
        : public infra::Subject<AttMtuExchangeObserver>
    {
    public:
        virtual uint16_t EffectiveMaxAttMtuSize() const = 0;

    protected:
        static constexpr uint16_t defaultMaxAttMtuSize = 23;
    };

    inline GattCharacteristic::PropertyFlags operator|(GattCharacteristic::PropertyFlags lhs, GattCharacteristic::PropertyFlags rhs)
    {
        return static_cast<GattCharacteristic::PropertyFlags>(infra::enum_cast(lhs) | infra::enum_cast(rhs));
    }

    inline GattCharacteristic::PropertyFlags operator&(GattCharacteristic::PropertyFlags lhs, GattCharacteristic::PropertyFlags rhs)
    {
        return static_cast<GattCharacteristic::PropertyFlags>(infra::enum_cast(lhs) & infra::enum_cast(rhs));
    }

    inline GattCharacteristic::PermissionFlags operator|(GattCharacteristic::PermissionFlags lhs, GattCharacteristic::PermissionFlags rhs)
    {
        return static_cast<GattCharacteristic::PermissionFlags>(infra::enum_cast(lhs) | infra::enum_cast(rhs));
    }
}

#endif
