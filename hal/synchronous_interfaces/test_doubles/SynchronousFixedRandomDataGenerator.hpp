#ifndef SYNCHRONOUS_HAL_FIXED_RANDOM_DATA_GENERATOR_MOCK_HPP
#define SYNCHRONOUS_HAL_FIXED_RANDOM_DATA_GENERATOR_MOCK_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include <vector>

namespace hal
{
    class SynchronousFixedRandomDataGenerator
        : public hal::SynchronousRandomDataGenerator
    {
    public:
        explicit SynchronousFixedRandomDataGenerator(const std::vector<uint8_t>& initialData);

        void GenerateRandomData(infra::ByteRange result);

    private:
        std::vector<uint8_t> initialData;
    };
}

#endif
