#include "upgrade/boot_loader/ImageUpgraderFlash.hpp"
#include "services/tracer/GlobalTracer.hpp"

namespace application
{
    ImageUpgraderFlash::ImageUpgraderFlash(infra::ByteRange buffer, const char* targetName, Decryptor& decryptor, hal::SynchronousFlash& flash, uint32_t destinationAddressOffset)
        : ImageUpgrader(targetName, decryptor)
        , buffer(buffer)
        , flash(&flash)
        , destinationAddressOffset(destinationAddressOffset)
    {
        services::GlobalTracer().Trace() << "ImageUpgraderFlash:: ctor";
    }

    uint32_t ImageUpgraderFlash::Upgrade(hal::SynchronousFlash& upgradePackFlash, uint32_t imageAddress, uint32_t imageSize, uint32_t destinationAddress)
    {
        destinationAddress += destinationAddressOffset;

        services::GlobalTracer().Trace() << "ImageUpgraderFlash:: Upgrade, erasing sectors...";

        flash->EraseSectors(flash->SectorOfAddress(destinationAddress), flash->SectorOfAddress(destinationAddress + imageSize - 1) + 1);

        services::GlobalTracer().Trace() << "ImageUpgraderFlash:: Upgrade, erased";

        uint32_t imageAddressStart = imageAddress;
        while (imageAddress - imageAddressStart < imageSize)
        {
            infra::ByteRange bufferRange(infra::Head(buffer, imageSize - (imageAddress - imageAddressStart)));
            upgradePackFlash.ReadBuffer(bufferRange, imageAddress);
            imageAddress += bufferRange.size();

            services::GlobalTracer().Trace() << "ImageUpgraderFlash:: Upgrade, image address: " << imageAddress;

            services::GlobalTracer().Trace() << "ImageUpgraderFlash:: Upgrade, decrypting chunk...";

            ImageDecryptor().DecryptPart(bufferRange);

            services::GlobalTracer().Trace() << "ImageUpgraderFlash:: Upgrade, decrypted...";

            flash->WriteBuffer(bufferRange, destinationAddress);

            services::GlobalTracer().Trace() << "ImageUpgraderFlash:: Upgrade, writing into flash...";

            destinationAddress += bufferRange.size();
        }

        return 0;
    }

    void ImageUpgraderFlash::SetFlash(hal::SynchronousFlash& flash)
    {
        services::GlobalTracer().Trace() << "ImageUpgraderFlash:: SetFlash";
        this->flash = &flash;
    }
}
