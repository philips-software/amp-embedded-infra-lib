#ifndef HAL_SYNCHRONOUS_RANDOM_DATA_GENERATOR_WIN_HPP
#define HAL_SYNCHRONOUS_RANDOM_DATA_GENERATOR_WIN_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include <windows.h>

namespace hal
{    
    class SynchronousRandomDataGeneratorWin
        : public SynchronousRandomDataGenerator
    {
    public:
        SynchronousRandomDataGeneratorWin();
        ~SynchronousRandomDataGeneratorWin();

        virtual void GenerateRandomData(infra::ByteRange result) override;

    private:
        HCRYPTPROV cryptoProvider = 0;
    };
}

#endif
