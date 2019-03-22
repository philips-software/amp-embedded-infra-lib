#ifndef UPGRADE_IMAGE_UPGRADER_ERASE_SECTORS_HPP
#define UPGRADE_IMAGE_UPGRADER_ERASE_SECTORS_HPP

#include "infra/util/WithStorage.hpp"
#include "upgrade/boot_loader/ImageUpgrader.hpp"

namespace application
{
    class ImageUpgraderEraseSectors
        : public ImageUpgrader
    {
    public:
        ImageUpgraderEraseSectors(const char* targetName, Decryptor& decryptor, hal::SynchronousFlash& internalFlash, uint32_t sectorStart, uint32_t sectorEnd);

        virtual uint32_t Upgrade(hal::SynchronousFlash& upgradePackFlash, uint32_t imageAddress, uint32_t imageSize, uint32_t destinationAddress) override;

    private:
        hal::SynchronousFlash* internalFlash;
        uint32_t sectorStart;
        uint32_t sectorEnd;
    };
}

#endif
