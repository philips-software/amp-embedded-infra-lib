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
        {
            for (std::size_t imageIndex = 0; imageIndex != headerEpilogue.numberOfImages; ++imageIndex)
                if (ImageFound(decryptor))
                    return true;
        }

        return false;
    }

    bool UpgradePackLoader::ImageFound(Decryptor& decryptor)
    {
        return true;
    }

    void UpgradePackLoader::MarkAsError(uint32_t errorCode)
    {
        static const UpgradePackStatus statusError = UpgradePackStatus::invalid;
        upgradePackFlash.WriteBuffer(infra::MakeByteRange(statusError), 0);
        upgradePackFlash.WriteBuffer(infra::MakeByteRange(errorCode), 4);
    }
}
