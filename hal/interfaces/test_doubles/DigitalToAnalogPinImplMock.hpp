#ifndef HAL_ANALOG_TO_DIGITAL_PIN_IMPL_MOCK_HPP
#define HAL_ANALOG_TO_DIGITAL_PIN_IMPL_MOCK_HPP

#include "gmock/gmock.h"
#include "hal/interfaces/DigitalToAnalogPin.hpp"

namespace hal
{
    class DigitalToAnalogPinImplMock
        : public DigitalToAnalogPinImplBase
    {
    public:
        MOCK_METHOD1(Set, void(uint16_t value));
    };
}

#endif
