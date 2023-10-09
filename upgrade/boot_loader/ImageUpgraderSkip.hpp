#ifndef UPGRADE_IMAGE_UPGRADER_SKIP_HPP
#define UPGRADE_IMAGE_UPGRADER_SKIP_HPP

#include "upgrade/boot_loader/DecryptorNone.hpp"
#include "upgrade/boot_loader/ImageUpgrader.hpp"

namespace application
{
    class ImageUpgraderSkip
        : public ImageUpgrader
    {
    public:
        explicit ImageUpgraderSkip(const char* targetName);

        uint32_t Upgrade(hal::SynchronousFlash& flash, uint32_t imageAddress, uint32_t imageSize, uint32_t destinationAddress) override;

    private:
        static DecryptorNone decryptorNone;
    };
}

#endif
