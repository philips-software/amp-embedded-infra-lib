#ifndef SYNCHRONOUS_HAL_RANDOM_DATA_GENERATOR_MOCK_HPP
#define SYNCHRONOUS_HAL_RANDOM_DATA_GENERATOR_MOCK_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "gmock/gmock.h"
#include <algorithm>
#include <random>

namespace hal
{
    class SynchronousRandomDataGeneratorMock
        : public hal::SynchronousRandomDataGenerator
    {
    public:
        MOCK_METHOD1(GenerateRandomData, void(infra::ByteRange result));
    };

    class SynchronousRandomDataGeneratorStub
        : public hal::SynchronousRandomDataGenerator
    {
    public:
        void GenerateRandomData(infra::ByteRange result) override
        {
            std::generate(result.begin(), result.end(), [this]()
                {
                    return distribution(generator);
                });
        }

        std::mt19937 generator;
        std::uniform_int_distribution<int> distribution{ 0, 255 };
    };
}

#endif
