#ifndef SERVICES_GATT_HPP
#define SERVICES_GATT_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/EnumCast.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/Variant.hpp"
#include <array>

namespace services
{
    struct AttAttribute
    {
        using Uuid16 = uint16_t;
        using Uuid128 = infra::BigEndian<std::array<uint8_t, 16>>;
        using Uuid = infra::Variant<Uuid16, Uuid128>;

        using Handle = uint16_t;
    };

    struct GattDescriptor
    {
        struct ClientCharacteristicConfiguration
        {
            static constexpr uint8_t valueHandleOffset = 1;
            static constexpr uint16_t attributeType = 0x2902;

            enum class CharacteristicValue : uint16_t
            {
                disable = 0x0000,
                enableNotification = 0x0001,
                enableIndication = 0x0002,
            };
        };
    };

    class GattCharacteristic
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

    public:
        GattCharacteristic(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, PropertyFlags properties);
        GattCharacteristic() = default;
        GattCharacteristic(GattCharacteristic& other) = delete;
        GattCharacteristic& operator=(const GattCharacteristic& other) = delete;
        virtual ~GattCharacteristic() = default;

        const PropertyFlags& Properties() const;
        const AttAttribute::Uuid& Type() const;

        AttAttribute::Handle Handle() const;
        AttAttribute::Handle& Handle();
        AttAttribute::Handle ValueHandle() const;
        AttAttribute::Handle& ValueHandle();

    protected:
        AttAttribute::Uuid type;
        AttAttribute::Handle handle;
        AttAttribute::Handle valueHandle;
        PropertyFlags properties;
    };

    class GattService
    {
    public:
        GattService(const AttAttribute::Uuid& type);
        GattService(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle);
        GattService(GattService& other) = delete;
        GattService& operator=(const GattService& other) = delete;
        virtual ~GattService() = default;

        AttAttribute::Uuid Type() const;
        AttAttribute::Handle Handle() const;
        AttAttribute::Handle& Handle();
        AttAttribute::Handle EndHandle() const;
        AttAttribute::Handle& EndHandle();
        uint8_t GetAttributeCount() const;

    protected:
        AttAttribute::Uuid type;
        AttAttribute::Handle handle;
        AttAttribute::Handle endHandle;
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
}

namespace infra
{
    TextOutputStream& operator<<(TextOutputStream& stream, const services::AttAttribute::Uuid& uuid);
    TextOutputStream& operator<<(TextOutputStream& stream, const services::GattCharacteristic::PropertyFlags& properties);
}

#endif
