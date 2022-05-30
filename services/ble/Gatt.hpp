#ifndef SERVICES_GATT_HPP
#define SERVICES_GATT_HPP

#include "infra/util/IntrusiveForwardList.hpp"
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
        Gatt::Handle& Handle() const;

    private:
        const Gatt::Uuid& type;
        mutable Gatt::Handle handle;
    };

    class GattCharacteristic
        : public infra::IntrusiveForwardList<GattCharacteristic>::NodeType
        , public GattIdentifier
    {
    public:
        using GattIdentifier::GattIdentifier;

        GattCharacteristic& operator=(const GattCharacteristic& other) = delete;
        GattCharacteristic(GattCharacteristic& other) = delete;

        GattCharacteristic& operator=(const char* data);

    private:
        Gatt::AccessPermission accessPermission{ Gatt::AccessPermission::none };
        Gatt::Encryption encryption{ Gatt::Encryption::none };
        bool AuthorizationRequired{ false };
    };

    class GattService
        : public GattIdentifier
    {
    public:
        using GattIdentifier::GattIdentifier;
        template<class... Args>
            GattService(const Gatt::Uuid& type, const Args&... characteristics);

        GattService& operator=(const GattService& other) = delete;
        GattService(GattService& other) = delete;

        void AddCharacteristic(const GattCharacteristic& characteristic);
        const infra::IntrusiveForwardList<GattCharacteristic>& Characteristics() const;

    private:
        template <std::size_t I = 0, typename... Ts>
        typename std::enable_if<I == sizeof...(Ts), void>::type
        AddCharacteristic(std::tuple<Ts...> element)
        {
            return;
        }

        template <std::size_t I = 0, typename... Ts>
        typename std::enable_if<(I < sizeof...(Ts)), void>::type
        AddCharacteristic(std::tuple<Ts...> element)
        {
            AddCharacteristic(std::get<I>(element));
            AddCharacteristic<I + 1>(element);
        }

    private:
        infra::IntrusiveForwardList<GattCharacteristic> characteristics;
    };

    template<class... Args>
    GattService::GattService(const Gatt::Uuid& type, const Args&... characteristics)
        : GattService(type)
    {
        AddCharacteristic(std::tuple<const Args&...>{characteristics...});
    }

    class GattServer
    {
    public:
        virtual void AddService(const GattService& service) = 0;
        virtual void AddCharacteristic(const GattService& service, const GattCharacteristic& characteristic) = 0;
    };
}

#endif
