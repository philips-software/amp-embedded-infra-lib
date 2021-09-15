#ifndef UPGRADE_PACK_UPGRADER_HPP
#define UPGRADE_PACK_UPGRADER_HPP

#include "hal/synchronous_interfaces/SynchronousFlash.hpp"
#include "infra/util/ByteRange.hpp"
#include "upgrade/boot_loader/ImageUpgrader.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"

namespace application
{
    class PackUpgrader
    {
    public:
        explicit PackUpgrader(hal::SynchronousFlash& upgradePackFlash);

        void UpgradeFromImages(infra::MemoryRange<ImageUpgrader*> imageUpgraders);
        bool HasImage(const char* imageName) const;

        void MarkAsError(uint32_t errorCode);

    private:
        bool TryUpgradeImage(infra::MemoryRange<ImageUpgrader*> imageUpgraders);
        bool IsImage(uint32_t& imageAddress, const char* imageName) const;
        void MarkAsDeployStarted();
        void MarkAsDeployed();

    private:
        hal::SynchronousFlash& upgradePackFlash;
        uint32_t address = 0;
    };
}

#endif
