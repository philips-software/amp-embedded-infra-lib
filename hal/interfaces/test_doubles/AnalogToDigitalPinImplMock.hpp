#ifndef HAL_ANALOG_TO_DIGITAL_PIN_IMPL_MOCK_HPP
#define HAL_ANALOG_TO_DIGITAL_PIN_IMPL_MOCK_HPP

#include "hal/interfaces/AnalogToDigitalPin.hpp"
#include "infra/util/Unit.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class AnalogToDigitalPinImplMock
        : AnalogToDigitalPinImplBase
    {
    public:
        MOCK_METHOD1(Measure, void(const infra::Function<void(int32_t value)>& onDone));
    };
}

#endif
