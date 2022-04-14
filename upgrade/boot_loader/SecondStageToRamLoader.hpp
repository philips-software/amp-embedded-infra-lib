#ifndef UPGRADE_SECOND_STAGE_TO_RAM_LOADER_HPP
#define UPGRADE_SECOND_STAGE_TO_RAM_LOADER_HPP

#include "hal/synchronous_interfaces/SynchronousFlash.hpp"
#include "upgrade/boot_loader/Decryptor.hpp"
#include "upgrade/boot_loader/Verifier.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"

namespace application
{
    class SecondStageToRamLoader
    {
    public:
        SecondStageToRamLoader(hal::SynchronousFlash& upgradePackFlash, const char* product);

        bool Load(infra::ByteRange ram, Decryptor& decryptor, const Verifier& verifier, bool override = false);
        void MarkAsError(uint32_t errorCode);

    protected:
        virtual UpgradePackStatus ReadStatus();
        virtual void WriteStatus(UpgradePackStatus status);
        virtual void WriteError(uint32_t errorCode);

    private:
        bool TryLoadImage(infra::ByteRange ram, Decryptor& decryptor);

    private:
        hal::SynchronousFlash& upgradePackFlash;
        const char* product;
        uint32_t address = 0;
    };
}

#endif
