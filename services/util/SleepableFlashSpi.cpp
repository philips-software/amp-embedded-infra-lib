#include "services/util/SleepableFlashSpi.hpp"

namespace services
{
    SleepableFlashSpi::SleepableFlashSpi(hal::SpiMaster& spi, const Config& config, uint32_t timerId)
        : FlashSpi(spi, config, timerId)
        , claimer(Resource())
    {
    }

    void SleepableFlashSpi::Sleep(const infra::Function<void()>& onDone)
    {}

    void SleepableFlashSpi::Wake(const infra::Function<void()>& onDone)
    {}
}
