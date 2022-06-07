#ifndef SERVICES_GATT_HPP
#define SERVICES_GATT_HPP

#include "infra/util/ByteRange.hpp"
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

        const Uuid& type;
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
        enum class AccessPermission : uint8_t
        {
            none,
            readable,
            writeable,
            readableAndWritable
        };

        enum class Encryption : uint8_t
        {
            none,
            unauthenticated,
            autenticated
        };

    public:
        GattCharacteristic() = default;
        GattCharacteristic(GattCharacteristic& other) = delete;
        GattCharacteristic& operator=(const GattCharacteristic& other) = delete;

        virtual GattAttribute::Uuid Type() const = 0;

        virtual GattAttribute::Handle Handle() const = 0;
        virtual GattAttribute::Handle& Handle() = 0;

        virtual uint16_t ValueLength() const = 0;

        virtual void Update(infra::ConstByteRange data, infra::Function<void()> onDone) = 0;
    };

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
