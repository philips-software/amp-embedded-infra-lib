#ifndef HAL_ANALOG_TO_DIGITAL_PIN_IMPL_MOCK_HPP
#define HAL_ANALOG_TO_DIGITAL_PIN_IMPL_MOCK_HPP

#include "hal/interfaces/AnalogToDigitalPin.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class AnalogToDigitalPinImplMock
        : public AnalogToDigitalPinImplBase<uint32_t>
    {
    public:
        virtual ~AnalogToDigitalPinImplMock() = default;

        MOCK_METHOD2(Measure, void(std::size_t numberOfSamples, const infra::Function<void(infra::MemoryRange<int32_t> samples)>& onDone));
    };
}

#endif
