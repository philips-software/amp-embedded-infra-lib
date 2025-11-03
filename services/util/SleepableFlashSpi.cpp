#include "services/util/SleepableFlashSpi.hpp"
#include "infra/util/ByteRange.hpp"

namespace services
{

    SleepableFlashSpi::SleepableFlashSpi(hal::SpiMaster& spi, const Config& config, uint32_t timerId)
        : FlashSpi(spi, config, timerId)
        , sleepClaimer(Resource())
        , wakeClaimer(Resource())
    {
    }

    void SleepableFlashSpi::Sleep(const infra::Function<void()>& onDone)
    {
        onSleepDone = onDone;

        sleepClaimer.Claim([this]()
            {
                Spi().SendData(infra::MakeByteRange(commandDeepPowerDown), hal::SpiAction::stop, [this]()
                    {
                        sleepClaimer.Release();
                        this->onSleepDone();
                    });
            });
    }

    void SleepableFlashSpi::Wake(const infra::Function<void()>& onDone)
    {
        onWakeDone = onDone;

        wakeClaimer.Claim([this]()
            {
                Spi().SendData(infra::MakeByteRange(commandReleaseFromDeepPowerDown), hal::SpiAction::stop, [this]()
                    {
                        wakeClaimer.Release();
                        this->onWakeDone();
                    });
            });
    }
}
