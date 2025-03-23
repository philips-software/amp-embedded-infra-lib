#ifndef MBED_TLS_RANDOM_DATA_GENERATOR_WRAPPER_HPP
#define MBED_TLS_RANDOM_DATA_GENERATOR_WRAPPER_HPP

#include <cstdlib>

namespace services
{
    int MbedTlsRandomDataGeneratorWrapper(void* data, unsigned char* output, std::size_t size);
}

#endif
