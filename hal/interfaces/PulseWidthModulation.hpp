#ifndef HAL_PULSE_WIDTH_MODULATION_HPP
#define HAL_PULSE_WIDTH_MODULATION_HPP

#include <cstdint>

namespace hal
{
    class PulseWidthModulation
    {
    protected:
        PulseWidthModulation() = default;
        PulseWidthModulation(const PulseWidthModulation&) = delete;
        PulseWidthModulation& operator=(const PulseWidthModulation&) = delete;
        ~PulseWidthModulation() = default;

    public:
        virtual void SetDuty(uint8_t dutyPercent) = 0;
        virtual void SetPulse(uint32_t pulseOn, uint32_t period) = 0;
        virtual void Start() = 0;
        virtual void Stop() = 0;
    };
}

#endif // HAL_PULSE_WIDTH_MODULATION_HPP
