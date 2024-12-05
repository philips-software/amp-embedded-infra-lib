#ifndef HAL_ADC_MULTI_CHANNEL_STUB_HPP
#define HAL_ADC_MULTI_CHANNEL_STUB_HPP

#include "hal/interfaces/test_doubles/AdcMultiChannelMock.hpp"
#include "gmock/gmock.h"
#include <cstdint>
#include <vector>

namespace hal
{
    class AdcMultiChannelStub
        : public AdcMultiChannelMock
    {
    public:
        AdcMultiChannelStub();

        void MeasurementDone(const std::vector<uint16_t>& samples);

    private:
        infra::Function<void(Samples)> onDone;
    };
}

#endif // HAL_ADC_MULTI_CHANNEL_STUB_HPP
