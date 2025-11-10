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
        static constexpr uint8_t commandDeepPowerDown = 0xB9;
        static constexpr uint8_t commandReleaseFromDeepPowerDown = 0xAB;

        explicit SleepableFlashSpi(hal::SpiMaster& spi, const Config& config = Config(), uint32_t timerId = infra::systemTimerServiceId);

        void Sleep(const infra::Function<void()>& onDone) override;
        void Wake(const infra::Function<void()>& onDone) override;

    private:
        infra::ClaimableResource::Claimer sleepClaimer;
        infra::ClaimableResource::Claimer wakeClaimer;
        infra::AutoResetFunction<void()> onSleepDone;
        infra::AutoResetFunction<void()> onWakeDone;
    };
}

#endif
