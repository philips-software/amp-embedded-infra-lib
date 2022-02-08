#ifndef UPGRADE_SECOND_STAGE_TO_RAM_LOADER_HPP
#define UPGRADE_SECOND_STAGE_TO_RAM_LOADER_HPP

#include "upgrade/boot_loader/UpgradePackLoader.hpp"

namespace application
{
    class SecondStageToRamLoader
        : public UpgradePackLoader
    {
    public:
        SecondStageToRamLoader(hal::SynchronousFlash& upgradePackFlash, const char* product, infra::ByteRange ram);

        virtual bool Load(Decryptor& decryptor, const Verifier& verifier);
        virtual bool ImageFound(Decryptor& decryptor) override;

    private:
        infra::ByteRange ram;
    };
}

#endif
