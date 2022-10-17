#include "services/util/Sha256MbedTls.hpp"
#include "infra/util/Compatibility.hpp"
#include "mbedtls/sha256.h"
#include "mbedtls/version.h"

#include <cassert>

#if MBEDTLS_VERSION_MAJOR < 3
#define mbedtls_sha256 mbedtls_sha256_ret
#endif

namespace services
{
    std::array<uint8_t, 32> Sha256MbedTls::Calculate(infra::ConstByteRange input) const
    {
        std::array<uint8_t, 32> output;

        EMIL_MAYBE_UNUSED auto result = mbedtls_sha256(input.begin(), input.size(), output.data(), 0);
        assert(result == 0);

        return output;
    }
}
