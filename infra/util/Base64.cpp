#include "infra/util/Base64.hpp"
#include <algorithm>

namespace infra
{
    namespace _detail
    {
        const char* base64EncodeTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    }

    uint8_t DecodeBase64Byte(char base64)
    {
        return static_cast<uint8_t>(std::find(&_detail::base64EncodeTable[0], &_detail::base64EncodeTable[64], base64) - &_detail::base64EncodeTable[0]);
    }
}
