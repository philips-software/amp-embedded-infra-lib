#ifndef SERVICES_GATT_HPP
#define SERVICES_GATT_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/EnumCast.hpp"
#include "infra/util/Observer.hpp"
#include "services/ble/Att.hpp"
#include <cstdint>

namespace services
{
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

        GattDescriptor(const AttAttribute::Uuid& type, AttAttribute::Handle handle);
        GattDescriptor() = default;

        const AttAttribute::Uuid& Type() const;

        AttAttribute::Handle Handle() const;
        AttAttribute::Handle& Handle();

        bool operator==(const GattDescriptor& other) const;
        bool operator!=(const GattDescriptor& other) const;

    private:
        AttAttribute::Uuid type;
        AttAttribute::Handle handle;
    };

    namespace uuid
    {
        constexpr inline AttAttribute::Uuid16 deviceInformationService{ 0x180A };
        constexpr inline AttAttribute::Uuid16 systemId{ 0x2A23 };
        constexpr inline AttAttribute::Uuid16 modelNumber{ 0x2A24 };
        constexpr inline AttAttribute::Uuid16 serialNumber{ 0x2A25 };
        constexpr inline AttAttribute::Uuid16 firmwareRevision{ 0x2A26 };
        constexpr inline AttAttribute::Uuid16 hardwareRevision{ 0x2A27 };
        constexpr inline AttAttribute::Uuid16 softwareRevision{ 0x2A28 };
        constexpr inline AttAttribute::Uuid16 manufacturerName{ 0x2A29 };
        constexpr inline AttAttribute::Uuid16 ieeeCertification{ 0x2A2A };
        constexpr inline AttAttribute::Uuid16 pnpId{ 0x2A50 };
    }

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
        using Observer::Observer;

        virtual void ExchangedAttMtuSize() = 0;
    };

    class AttMtuExchange
        : public infra::Subject<AttMtuExchangeObserver>
    {
    protected:
        ~AttMtuExchange() = default;

    public:
        virtual uint16_t EffectiveAttMtuSize() const = 0;
    };

    class AttMtuExchangeImpl
        : public AttMtuExchange
    {
    public:
        virtual ~AttMtuExchangeImpl() = default;
        uint16_t EffectiveAttMtuSize() const override;
        void SetAttMtu(uint16_t value);

        static constexpr uint16_t defaultAttMtuSize = 23;

    private:
        uint16_t attMtu = defaultAttMtuSize;
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
