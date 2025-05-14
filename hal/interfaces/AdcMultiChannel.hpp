#ifndef HAL_ADC_MULTI_CHANNEL_HPP
#define HAL_ADC_MULTI_CHANNEL_HPP

#include "infra/util/Function.hpp"
#include "infra/util/MemoryRange.hpp"
#include <cstdint>

namespace hal
{
    class AdcMultiChannel
    {
    public:
        using Samples = infra::MemoryRange<const uint16_t>;

        virtual void Measure(const infra::Function<void(Samples)>& onDone) = 0;
        virtual void Stop() = 0;

    protected:
        AdcMultiChannel() = default;
        AdcMultiChannel(const AdcMultiChannel& other) = delete;
        AdcMultiChannel& operator=(const AdcMultiChannel& other) = delete;
        ~AdcMultiChannel() = default;
    };
}

#endif // HAL_ADC_MULTI_CHANNEL_HPP
