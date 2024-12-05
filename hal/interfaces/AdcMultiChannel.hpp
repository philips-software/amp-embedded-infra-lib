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

    protected:
        ~AdcMultiChannel() = default;
    };
}

#endif // HAL_ADC_MULTI_CHANNEL_HPP
