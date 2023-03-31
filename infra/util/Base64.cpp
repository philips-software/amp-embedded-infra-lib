#include "infra/util/Base64.hpp"
#include <algorithm>

namespace infra
{
    namespace detail
    {
        const char* base64Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    }

    uint8_t DecodeBase64Byte(char base64)
    {
        return static_cast<uint8_t>(std::find(&detail::base64Table[0], &detail::base64Table[64], base64) - &detail::base64Table[0]);
    }
}
