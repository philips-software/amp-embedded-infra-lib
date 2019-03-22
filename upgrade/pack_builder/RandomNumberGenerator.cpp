#include "upgrade/pack_builder/RandomNumberGenerator.hpp"
#include <algorithm>

namespace application
{
    SecureRandomNumberGenerator::SecureRandomNumberGenerator()
    {
        if (!::CryptAcquireContextW(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
            throw std::exception("Could not acquire crypt context");
    }

    SecureRandomNumberGenerator::~SecureRandomNumberGenerator()
    {
        ::CryptReleaseContext(hProvider, 0);
    }

    std::vector<uint8_t> SecureRandomNumberGenerator::Generate(std::size_t n)
    {
        std::vector<uint8_t> result(n, 0);

        if (!::CryptGenRandom(hProvider, n, result.data()))
            throw std::exception("Could not generate random data");

        return result;
    }

    FixedRandomNumberGenerator::FixedRandomNumberGenerator(const std::vector<uint8_t>& initialData)
        : initialData(initialData)
    {}

    std::vector<uint8_t> FixedRandomNumberGenerator::Generate(std::size_t n)
    {
        std::vector<uint8_t> result(initialData.begin(), initialData.begin() + std::min(initialData.size(), n));
        initialData.resize(initialData.size() - result.size());
        result.resize(n, 0);
        return result;
    }
}
