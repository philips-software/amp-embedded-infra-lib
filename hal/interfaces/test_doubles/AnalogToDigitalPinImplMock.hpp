#ifndef HAL_ANALOG_TO_DIGITAL_PIN_IMPL_MOCK_HPP
#define HAL_ANALOG_TO_DIGITAL_PIN_IMPL_MOCK_HPP

#include "hal/interfaces/AnalogToDigitalPin.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class AnalogToDigitalPinImplMock
        : public AnalogToDigitalPinImplBase
    {
    public:
        virtual ~AnalogToDigitalPinImplMock() = default;

        MOCK_METHOD2(Measure, void(infra::MemoryRange<int32_t> samples, const infra::Function<void()>& onDone));
    };
}

#endif
