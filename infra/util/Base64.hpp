#ifndef INFRA_BASE64_HPP
#define INFRA_BASE64_HPP

#include <cstdint>

namespace infra
{
    namespace detail
    {
        extern const char* base64Table;
    }

    uint8_t DecodeBase64Byte(char base64);
}

#endif
