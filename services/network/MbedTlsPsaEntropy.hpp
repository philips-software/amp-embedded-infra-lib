#ifndef SERVICES_MBED_TLS_PSA_ENTROPY_HPP
#define SERVICES_MBED_TLS_PSA_ENTROPY_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "psa/crypto.h"

extern "C"
{
    static hal::SynchronousRandomDataGenerator* psaRng = NULL;

    psa_status_t mbedtls_psa_external_get_random(mbedtls_psa_external_random_context_t* context, uint8_t* output, size_t output_size, size_t* output_length)
    {
        if (psaRng == NULL)
            return PSA_ERROR_BAD_STATE;
        psaRng->GenerateRandomData(infra::ByteRange(output, output + output_size));
        *output_length = output_size;
        return PSA_SUCCESS;
    }

    void psa_external_rng_init(hal::SynchronousRandomDataGenerator* rng)
    {
        psaRng = rng;
    }
}
#endif
