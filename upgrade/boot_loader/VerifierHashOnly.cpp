#include "mbedtls/sha256.h"
#include "upgrade/boot_loader/VerifierHashOnly.hpp"

namespace application
{
    bool VerifierHashOnly::IsValid(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const
    {
        std::array<uint8_t, 32> messageHash = Hash(flash, data);
        std::array<uint8_t, 32> storedHash;

        if (signature.second - signature.first != storedHash.size())
            return false;

        flash.ReadBuffer(storedHash, signature.first);

        return messageHash == storedHash;
    }

    std::array<uint8_t, 32> VerifierHashOnly::Hash(hal::SynchronousFlash& flash, const hal::SynchronousFlash::Range& data) const
    {
        std::array<uint8_t, 32> hash;

        mbedtls_sha256_context ctx;
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts(&ctx, 0);

        auto message = data.first;
        while (message != data.second)
        {
            std::array<uint8_t, 256> buffer;
            auto size = std::min<std::size_t>(buffer.size(), data.second - message);
            flash.ReadBuffer(infra::ByteRange(buffer.data(), buffer.data() + size), message);
            mbedtls_sha256_update(&ctx, buffer.data(), size);
            message += size;
        }

        mbedtls_sha256_finish(&ctx, hash.data());
        mbedtls_sha256_free(&ctx);

        return hash;
    }
}
