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
    class Gatt
    {
    public:
        using Uuid16 = uint16_t;
        using Uuid32 = uint32_t;
        using Uuid128 = std::array<uint8_t, 16>;
        using Uuid = infra::Variant<services::Gatt::Uuid16, services::Gatt::Uuid32, services::Gatt::Uuid128>;

        using Handle = uint16_t;

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
    };

    class GattIdentifier
    {
    public:
        explicit GattIdentifier(const Gatt::Uuid& type);

        GattIdentifier(GattIdentifier& other) = delete;
        GattIdentifier& operator=(const GattIdentifier& other) = delete;

        const Gatt::Uuid& Type() const;

        Gatt::Handle& Handle();
        Gatt::Handle Handle() const;

    private:
        const Gatt::Uuid& type;
        Gatt::Handle handle;
    };

    class GattCharacteristicClientOperations;
    class GattService;

    class GattCharacteristic
        : public infra::IntrusiveForwardList<GattCharacteristic>::NodeType
        , public infra::Observer<GattCharacteristic, GattCharacteristicClientOperations>
        , public GattIdentifier
    {
    public:
        GattCharacteristic(GattService& service, const Gatt::Uuid& type, uint16_t valueLength);

        void Update(infra::ConstByteRange data, infra::Function<void()> onDone);

        Gatt::Handle ServiceHandle() const;
        uint16_t ValueLength() const;

    private:
        const GattService& service;
        uint16_t valueLength;
        Gatt::AccessPermission accessPermission{ Gatt::AccessPermission::none };
        Gatt::Encryption encryption{ Gatt::Encryption::none };
        bool AuthorizationRequired{ false };
    };

    class GattService
        : public GattIdentifier
    {
    public:
        using GattIdentifier::GattIdentifier;

        void AddCharacteristic(const GattCharacteristic& characteristic);
        infra::IntrusiveForwardList<GattCharacteristic>& Characteristics();

    private:
        infra::IntrusiveForwardList<GattCharacteristic> characteristics;
    };

    class GattServer
    {
    public:
        virtual void AddService(GattService& service) = 0;
    };

    class GattCharacteristicClientOperations
        : public infra::Subject<GattCharacteristic>
    {
    public:
        // Update 'characteristic' with 'data' towards the
        // BLE stack and, depending on the configuration of
        // that 'characteristic', send a notification or indication.
        // Returns true on success, or false on failure (i.e. BLE
        // stack indicates an issue with updating or sending data).
        virtual bool Update(const GattCharacteristic& characteristic, infra::ConstByteRange data) const = 0;
    };
}

#endif
