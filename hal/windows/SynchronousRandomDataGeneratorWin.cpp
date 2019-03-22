#include "hal/windows/SynchronousRandomDataGeneratorWin.hpp"

namespace hal
{
    SynchronousRandomDataGeneratorWin::SynchronousRandomDataGeneratorWin()
    {
        if (!::CryptAcquireContextW(&cryptoProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
            throw std::exception("Could not acquire crypt context");
    }

    SynchronousRandomDataGeneratorWin::~SynchronousRandomDataGeneratorWin()
    {
        ::CryptReleaseContext(cryptoProvider, 0);
    }

    void SynchronousRandomDataGeneratorWin::GenerateRandomData(infra::ByteRange result)
    {
        if (!::CryptGenRandom(cryptoProvider, result.size(), result.begin()))
            throw std::exception("Could not generate random data");
    }
}
