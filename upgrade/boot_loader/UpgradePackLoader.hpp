#ifndef UPGRADE_UPGRADE_PACK_LOADER_HPP
#define UPGRADE_UPGRADE_PACK_LOADER_HPP

#include "hal/synchronous_interfaces/SynchronousFlash.hpp"
#include "upgrade/boot_loader/Decryptor.hpp"
#include "upgrade/boot_loader/Verifier.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"

namespace application
{
    class UpgradePackLoader
    {
    public:
        UpgradePackLoader(hal::SynchronousFlash& upgradePackFlash, const char* product);

        virtual bool Load(Decryptor& decryptor, const Verifier& verifier);
        virtual bool PostLoadActions(uint32_t numberOfImages, Decryptor& decryptor);

    protected:
        void MarkAsError(uint32_t errorCode);
        virtual UpgradePackStatus ReadStatus();
        virtual void WriteStatus(UpgradePackStatus status);
        virtual void WriteError(uint32_t errorCode);

    protected:
        hal::SynchronousFlash& upgradePackFlash;
        const char* product;
        uint32_t address = 0;
    };
}

#endif
