#include "upgrade/boot_loader/ImageUpgraderEraseSectors.hpp"

namespace application
{
    ImageUpgraderEraseSectors::ImageUpgraderEraseSectors(const char* targetName, Decryptor& decryptor, hal::SynchronousFlash& internalFlash, uint32_t sectorStart, uint32_t sectorEnd)
        : ImageUpgrader(targetName, decryptor)
        , internalFlash(&internalFlash)
        , sectorStart(sectorStart)
        , sectorEnd(sectorEnd)
    {}

    uint32_t ImageUpgraderEraseSectors::Upgrade(hal::SynchronousFlash& upgradePackFlash, uint32_t imageAddress, uint32_t imageSize, uint32_t destinationAddress)
    {
        internalFlash->EraseSectors(sectorStart, sectorEnd);

        return 0;
    }
}
