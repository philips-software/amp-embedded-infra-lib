#ifndef HAL_SYNCHRONOUS_RANDOM_DATA_GENERATOR_GENERIC_HPP
#define HAL_SYNCHRONOUS_RANDOM_DATA_GENERATOR_GENERIC_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include <random>

namespace hal
{    
    class SynchronousRandomDataGeneratorGeneric
        : public SynchronousRandomDataGenerator
    {
    public:
        virtual void GenerateRandomData(infra::ByteRange result) override;

    private:
        std::random_device randomDevice;
        std::uniform_int_distribution<int> distribution;
    };
}

#endif
