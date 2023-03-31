#ifndef INFRA_BASE64_HPP
#define INFRA_BASE64_HPP
#include <cstdint>

namespace infra
{
    namespace _detail
    {
        extern const char* base64EncodeTable;
    }

    uint8_t DecodeBase64Byte(char base64);
}

#endif
