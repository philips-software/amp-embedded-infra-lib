#ifndef SERVICES_SLEEPABLE_FLASH_SPI
#define SERVICES_SLEEPABLE_FLASH_SPI

#include "hal/interfaces/Sleepable.hpp"
#include "infra/event/ClaimableResource.hpp"
#include "services/util/FlashSpi.hpp"

namespace services
{
    class SleepableFlashSpi
        : public FlashSpi
        , public hal::Sleepable
    {
    public:
        SleepableFlashSpi(hal::SpiMaster& spi, const Config& config = Config(), uint32_t timerId = infra::systemTimerServiceId);

        void Sleep(const infra::Function<void()>& onDone) override;
        void Wake(const infra::Function<void()>& onDone) override;

    private:
        infra::ClaimableResource::Claimer claimer;
    };
}

#endif
