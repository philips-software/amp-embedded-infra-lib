#ifndef HAL_ADC_MULTI_CHANNEL_MOCK_HPP
#define HAL_ADC_MULTI_CHANNEL_MOCK_HPP

#include "hal/interfaces/PulseWidthModulation.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class PulseWidthModulationMock
        : public PulseWidthModulation
    {
    public:
        MOCK_METHOD(void, SetDuty, (uint8_t dutyPercent), (override));
        MOCK_METHOD(void, SetPulse, (uint32_t pulseOn, uint32_t period), (override));
        MOCK_METHOD(void, Start, (), (override));
        MOCK_METHOD(void, Stop, (), (override));
    };
}

#endif // HAL_ADC_MULTI_CHANNEL_MOCK_HPP
