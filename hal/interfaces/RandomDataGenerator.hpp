#ifndef HAL_RANDOM_DATA_GENERATOR_HPP
#define HAL_RANDOM_DATA_GENERATOR_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"

namespace hal
{
    class RandomDataGenerator
    {
    protected:
        RandomDataGenerator() = default;
        RandomDataGenerator(const RandomDataGenerator& other) = delete;
        RandomDataGenerator& operator=(const RandomDataGenerator& other) = delete;
        ~RandomDataGenerator() = default;

    public:
        virtual void GenerateRandomData(infra::ByteRange result, const infra::Function<void()>& onDone) = 0;
    };
}

#endif
