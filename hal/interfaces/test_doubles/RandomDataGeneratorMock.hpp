#ifndef HAL_RANDOM_DATA_GENERATOR_MOCK_HPP
#define HAL_RANDOM_DATA_GENERATOR_MOCK_HPP

#include "hal/interfaces/RandomDataGenerator.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class RandomDataGeneratorMock
        : public hal::RandomDataGenerator
    {
    public:
        MOCK_METHOD2(GenerateRandomData, void(infra::ByteRange result, const infra::Function<void()>& onDone));
    };
}

#endif
