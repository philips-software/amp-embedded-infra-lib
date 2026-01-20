#ifndef SERVICES_ATT_HPP
#define SERVICES_ATT_HPP

#include "infra/util/Endian.hpp"
#include <array>
#include <variant>

namespace services
{
    struct AttAttribute
    {
        using Uuid16 = uint16_t;
        using Uuid128 = infra::BigEndian<std::array<uint8_t, 16>>;
        using Uuid = std::variant<Uuid16, Uuid128>;

        using Handle = uint16_t;
    };
}

#endif
