#include "services/network/MbedTlsPsaEntropy.hpp"
#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/util/ReallyAssert.hpp"

#ifndef EMIL_HOST_BUILD

#include "psa/crypto.h"

static void GenerateRandomData(void* rng, uint8_t* output, size_t output_size)
{
    reinterpret_cast<hal::SynchronousRandomDataGenerator*>(rng)->GenerateRandomData(infra::ByteRange(output, output + output_size));
}

#ifdef __cplusplus
extern "C"
{
#endif
    static void* psaRng = NULL;

    psa_status_t mbedtls_psa_external_get_random(mbedtls_psa_external_random_context_t* context, uint8_t* output, size_t output_size, size_t* output_length)
    {
        really_assert(psaRng != NULL);
        GenerateRandomData(psaRng, output, output_size);
        *output_length = output_size;
        return PSA_SUCCESS;
    }

    void psa_external_rng_init(void* rng)
    {
        psaRng = rng;
    }

#ifdef __cplusplus
}
#endif

#endif
