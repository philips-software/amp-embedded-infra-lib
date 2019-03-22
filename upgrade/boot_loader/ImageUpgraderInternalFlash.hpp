#ifndef UPGRADE_IMAGE_UPGRADER_INTERNAL_FLASH_HPP
#define UPGRADE_IMAGE_UPGRADER_INTERNAL_FLASH_HPP

#include "infra/util/WithStorage.hpp"
#include "upgrade/boot_loader/ImageUpgrader.hpp"

namespace application
{
    class ImageUpgraderInternalFlash
        : public ImageUpgrader
    {
    public:
        template<std::size_t Size>
            using WithBlockSize = infra::WithStorage<ImageUpgraderInternalFlash, std::array<uint8_t, Size>>;

        ImageUpgraderInternalFlash(infra::ByteRange buffer, const char* targetName, Decryptor& decryptor, hal::SynchronousFlash& internalFlash, uint32_t destinationAddressOffset);

        virtual uint32_t Upgrade(hal::SynchronousFlash& upgradePackFlash, uint32_t imageAddress, uint32_t imageSize, uint32_t destinationAddress) override;

        void SetFlash(hal::SynchronousFlash& flash);

    private:
        infra::ByteRange buffer;
        hal::SynchronousFlash* internalFlash;
        uint32_t destinationAddressOffset;
    };
}

#endif
