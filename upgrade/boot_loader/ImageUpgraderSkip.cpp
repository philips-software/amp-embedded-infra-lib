#include "upgrade/boot_loader/ImageUpgraderSkip.hpp"
#include "services/tracer/GlobalTracer.hpp"

namespace application
{
    DecryptorNone ImageUpgraderSkip::decryptorNone;

    ImageUpgraderSkip::ImageUpgraderSkip(const char* targetName)
        : ImageUpgrader(targetName, decryptorNone)
    {
        services::GlobalTracer().Trace() << "ImageUpgraderSkip:: ctor";
    }

    uint32_t ImageUpgraderSkip::Upgrade(hal::SynchronousFlash& flash, uint32_t imageAddress, uint32_t imageSize, uint32_t destinationAddress)
    {
        services::GlobalTracer().Trace() << "ImageUpgraderSkip:: Upgrade";
        return 0;
    }
}
