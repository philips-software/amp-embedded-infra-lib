#include "upgrade/boot_loader/ImageUpgraderSkip.hpp"

namespace application
{
    DecryptorNone ImageUpgraderSkip::decryptorNone;

    ImageUpgraderSkip::ImageUpgraderSkip(const char* targetName)
        : ImageUpgrader(targetName, decryptorNone)
    {}

    uint32_t ImageUpgraderSkip::Upgrade(hal::SynchronousFlash& flash, uint32_t imageAddress, uint32_t imageSize, uint32_t destinationAddress)
    {
        return 0;
    }
}
