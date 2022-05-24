#ifndef SERVICES_GATT_HPP
#define SERVICES_GATT_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
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

    class GattCharacteristic
        : public infra::IntrusiveForwardList<GattCharacteristic>::NodeType
    {
    public:
        template<std::size_t StorageSize>
            using WithStorage = infra::WithStorage<GattCharacteristic, std::array<uint8_t, StorageSize>>;

        explicit GattCharacteristic(const Gatt::Uuid& type);
        GattCharacteristic(infra::ByteRange value, const Gatt::Uuid& type);
        GattCharacteristic& operator=(const GattCharacteristic& other) = delete;
        GattCharacteristic(GattCharacteristic& other) = delete;

    private:
        infra::ByteRange value;
        const Gatt::Uuid& type;
        Gatt::Handle handle;
        Gatt::AccessPermission accessPermission{ Gatt::AccessPermission::none };
        Gatt::Encryption encryption{ Gatt::Encryption::none };
        bool AuthorizationRequired{ false };
    };

    class GattService
    {
    public:
        explicit GattService(const Gatt::Uuid& type);
        template<class... Characteristics>
            GattService(const Gatt::Uuid& type, const Characteristics&... characteristics);
        GattService& operator=(const GattService& other) = delete;
        GattService(GattService& other) = delete;

        void AddCharacteristic(const GattCharacteristic& characteristic);

    private:
        template <size_t I = 0, typename... Ts>
        typename std::enable_if<I == sizeof...(Ts), void>::type
        AddCharacteristic(std::tuple<Ts...> element)
        {
            return;
        }

        template <size_t I = 0, typename... Ts>
        typename std::enable_if<(I < sizeof...(Ts)), void>::type
        AddCharacteristic(std::tuple<Ts...> element)
        {
            AddCharacteristic(std::get<I>(element));
            AddCharacteristic<I + 1>(element);
        }

    private:
        const Gatt::Uuid& type;
        Gatt::Handle handle;
        infra::IntrusiveForwardList<GattCharacteristic> characteristics;
    };

    template<class... Characteristics>
    GattService::GattService(const Gatt::Uuid& type, const Characteristics&... characteristics)
        : GattService(type)
    {
        AddCharacteristic(std::tuple<const Characteristics&...>{characteristics...});
    }
}

#endif
