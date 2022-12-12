#ifndef SYNCHRONOUS_HAL_RANDOM_DATA_GENERATOR_MOCK_HPP
#define SYNCHRONOUS_HAL_RANDOM_DATA_GENERATOR_MOCK_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class SynchronousRandomDataGeneratorMock
        : public hal::SynchronousRandomDataGenerator
    {
    public:
        MOCK_METHOD1(GenerateRandomData, void(infra::ByteRange result));
    };
}

#endif
