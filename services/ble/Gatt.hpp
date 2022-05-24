#ifndef SERVICES_GATT_HPP
#define SERVICES_GATT_HPP

#include "infra/util/IntrusiveList.hpp"
#include "infra/util/Variant.hpp"
#include "infra/util/WithStorage.hpp"

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
    };

    class GattCharacteristic
        : public infra::IntrusiveList<GattCharacteristic>::NodeType
    {
    public:
        GattCharacteristic(const Gatt::Uuid& uuid);
        GattCharacteristic& operator=(const GattCharacteristic& other) = delete;
        GattCharacteristic(GattCharacteristic& other) = delete;

    private:
        const Gatt::Uuid& uuid;
    };

    class GattService
    {
    public:
        GattService(const Gatt::Uuid& uuid, std::initializer_list<GattCharacteristic> characteristics);
        GattService& operator=(const GattService& other) = delete;
        GattService(GattService& other) = delete;

        void AddCharacteristic(const GattCharacteristic& characteristic);

    private:
        const Gatt::Uuid& uuid;
        infra::IntrusiveList<GattCharacteristic> characteristics;
    };
}

#endif
