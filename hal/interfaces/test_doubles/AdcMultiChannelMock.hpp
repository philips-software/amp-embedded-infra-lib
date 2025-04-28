#ifndef HAL_ADC_MULTI_CHANNEL_MOCK_HPP
#define HAL_ADC_MULTI_CHANNEL_MOCK_HPP

#include "hal/interfaces/AdcMultiChannel.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class AdcMultiChannelMock
        : public AdcMultiChannel
    {
    public:
        MOCK_METHOD(void, Measure, (const infra::Function<void(Samples)>& onDone), (override));
        MOCK_METHOD(void, Stop, (), (override));
    };
}

#endif // HAL_ADC_MULTI_CHANNEL_MOCK_HPP
