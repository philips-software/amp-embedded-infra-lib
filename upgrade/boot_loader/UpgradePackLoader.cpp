#include "infra/util/Compatibility.hpp"
#include "upgrade/boot_loader/UpgradePackLoader.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"
#include <cstring>

namespace application
{
    UpgradePackLoader::UpgradePackLoader(hal::SynchronousFlash& upgradePackFlash, const char* product)
        : upgradePackFlash(upgradePackFlash)
        , product(product)
    {}

    bool UpgradePackLoader::Load(Decryptor& decryptor, const Verifier& verifier)
    {
        UpgradePackHeaderPrologue headerPrologue;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(headerPrologue), address);
        headerPrologue.status = ReadStatus();
        address += sizeof(UpgradePackHeaderPrologue);

        bool isSane = (headerPrologue.status == UpgradePackStatus::readyToDeploy ||
                       headerPrologue.status == UpgradePackStatus::deployStarted) && headerPrologue.magic == upgradePackMagic;

        if (!isSane)
            return false;

        hal::SynchronousFlash::Range signature(address, address + headerPrologue.signatureLength);
        address += headerPrologue.signatureLength;

        hal::SynchronousFlash::Range signedContents(address, address + headerPrologue.signedContentsLength);

        UpgradePackHeaderEpilogue headerEpilogue;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(headerEpilogue), address);
        address += sizeof(UpgradePackHeaderEpilogue);

        if (headerEpilogue.headerVersion != 1)
            MarkAsError(upgradeErrorCodeUnknownHeaderVersion);
        else if (std::strcmp(product, headerEpilogue.productName.data()) != 0)
            MarkAsError(upgradeErrorCodeUnknownProductName);
        else if (!verifier.IsValid(upgradePackFlash, signature, signedContents))
            MarkAsError(upgradeErrorCodeInvalidSignature);
        else
            return PostLoadActions(headerEpilogue.numberOfImages, decryptor);

        return false;
    }

    bool UpgradePackLoader::PostLoadActions(EMIL_MAYBE_UNUSED uint32_t numberOfImages, EMIL_MAYBE_UNUSED Decryptor& decryptor)
    {
        return true;
    }

    void UpgradePackLoader::MarkAsError(uint32_t errorCode)
    {
        WriteStatus(UpgradePackStatus::invalid);
        WriteError(errorCode);
    }

    UpgradePackStatus UpgradePackLoader::ReadStatus()
    {
        UpgradePackStatus status;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(status), 0);
        return status;
    }

    void UpgradePackLoader::WriteStatus(UpgradePackStatus status)
    {
        upgradePackFlash.WriteBuffer(infra::MakeByteRange(status), 0);
    }

    void UpgradePackLoader::WriteError(uint32_t errorCode)
    {
        upgradePackFlash.WriteBuffer(infra::MakeByteRange(errorCode), 4);
    }
}
