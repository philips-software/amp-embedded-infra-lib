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
        virtual AttAttribute::Handle ServiceGroupHandle() const = 0;
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

    class GattCharacteristic
        : public GattCharacteristicClientOperationsObserver
        , public GattCharacteristicUpdate
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
        virtual uint8_t GetAttributeCount() const = 0;

        virtual AttAttribute::Uuid Type() const = 0;
        virtual AttAttribute::Handle Handle() const = 0;
        virtual AttAttribute::Handle& Handle() = 0;
    };

    class GattService
    {
    public:
        GattService(const AttAttribute::Uuid& type)
            : GattService(type, 0, 0)
        {}

        GattService(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle)
            : type(type)
            , handle(handle)
            , endHandle(endHandle)
        {}

        GattService(GattService& other) = delete;
        GattService& operator=(const GattService& other) = delete;
        virtual ~GattService() = default;

        AttAttribute::Uuid Type() const { return type; }
        AttAttribute::Handle Handle() const { return handle; }
        AttAttribute::Handle& Handle() { return handle; }
        AttAttribute::Handle EndHandle() const { return endHandle; }
        AttAttribute::Handle& EndHandle() { return endHandle; }
        uint8_t GetAttributeCount() const {}

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
