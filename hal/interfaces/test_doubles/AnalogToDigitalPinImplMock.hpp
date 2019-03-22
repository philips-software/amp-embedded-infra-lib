#ifndef HAL_ANALOG_TO_DIGITAL_PIN_IMPL_MOCK_HPP
#define HAL_ANALOG_TO_DIGITAL_PIN_IMPL_MOCK_HPP

#include "gmock/gmock.h"
#include "hal/interfaces/AnalogToDigitalPin.hpp"
#include "infra/util/Unit.hpp"

namespace hal
{
    class AnalogToDigitalPinImplMock
        : AnalogToDigitalPinImplBase
    {
    public:
        MOCK_METHOD1(Measure, void(const infra::Function<void(uint16_t value)>& onDone));
    };
}

#endif
