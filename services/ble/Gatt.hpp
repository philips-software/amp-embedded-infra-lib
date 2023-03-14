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
    struct AttAttribute
    {
        using Uuid16 = uint16_t;
        using Uuid128 = std::array<uint8_t, 16>;
        using Uuid = infra::Variant<Uuid16, Uuid128>;

        using Handle = uint16_t;

        Uuid type;
        Handle handle;
        Handle endHandle;
    };

    class GattDescriptor
    {
    public:
        explicit GattDescriptor(AttAttribute::Uuid& type, AttAttribute::Handle& handle);

        AttAttribute::Uuid& Type() const;
        AttAttribute::Handle& Handle() const;

    private:
        AttAttribute::Uuid& type;
        AttAttribute::Handle& handle;
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
        GattCharacteristic() = default;
        GattCharacteristic(GattCharacteristic& other) = delete;
        GattCharacteristic& operator=(const GattCharacteristic& other) = delete;
        virtual ~GattCharacteristic() = default;

        virtual PropertyFlags Properties() const = 0;

        virtual AttAttribute::Uuid Type() const = 0;
        virtual AttAttribute::Handle Handle() const = 0;
        virtual AttAttribute::Handle& Handle() = 0;
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

#endif
