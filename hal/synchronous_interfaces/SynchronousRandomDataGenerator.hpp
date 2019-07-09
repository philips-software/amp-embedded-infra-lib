#ifndef SYNCHRONOUS_HAL_RANDOM_DATA_GENERATOR_HPP
#define SYNCHRONOUS_HAL_RANDOM_DATA_GENERATOR_HPP

#include "infra/util/ByteRange.hpp"

namespace hal
{
    class SynchronousRandomDataGenerator
    {
    protected:
        SynchronousRandomDataGenerator() = default;
        SynchronousRandomDataGenerator(const SynchronousRandomDataGenerator& other) = delete;
        SynchronousRandomDataGenerator& operator=(const SynchronousRandomDataGenerator& other) = delete;
        ~SynchronousRandomDataGenerator() = default;

    public:
        virtual void GenerateRandomData(infra::ByteRange result) = 0;

        template<class T>
            T GenerateRandomData();
    };

    template<class T>
    T SynchronousRandomDataGenerator::GenerateRandomData()
    {
        T result;
        GenerateRandomData(infra::MakeByteRange(result));
        return result;
    }
}

#endif
