#include "services/network/MbedTlsPsaEntropy.hpp"
#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/util/ReallyAssert.hpp"

#ifndef EMIL_HOST_BUILD

#include "psa/crypto.h"

namespace
{
    int RandomDataGeneratorWrapper(void* data, unsigned char* output, std::size_t size)
    {
        reinterpret_cast<hal::SynchronousRandomDataGenerator*>(data)->GenerateRandomData(infra::ByteRange(output, output + size));
        return 0;
    }
}

#ifdef __cplusplus
extern "C"
{
#endif
    static void* psaRng = NULL;

    void psa_external_rng_init(void* rng)
    {
        psaRng = rng;
    }

    psa_status_t mbedtls_psa_external_get_random(mbedtls_psa_external_random_context_t* context, uint8_t* output, size_t output_size, size_t* output_length)
    {
        really_assert(psaRng != NULL);

        RandomDataGeneratorWrapper(psaRng, output, output_size);
        *output_length = output_size;

        return PSA_SUCCESS;
    }

#ifdef __cplusplus
}
#endif

#endif
