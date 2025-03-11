#ifndef SERVICES_MBED_TLS_PSA_ENTROPY_HPP
#define SERVICES_MBED_TLS_PSA_ENTROPY_HPP

#ifndef EMIL_HOST_BUILD

#ifdef __cplusplus
extern "C"
{
#endif

    void psa_external_rng_init(void* rng);

#ifdef __cplusplus
}
#endif

#endif

#endif
