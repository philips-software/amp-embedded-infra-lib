#include "upgrade/boot_loader/SecondStageToRamLoader.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"
#include <cstring>

namespace application
{
    SecondStageToRamLoader::SecondStageToRamLoader(hal::SynchronousFlash& upgradePackFlash, const char* product)
        : upgradePackFlash(upgradePackFlash)
        , product(product)
    {}

    bool SecondStageToRamLoader::Load(infra::ByteRange ram, Decryptor& decryptor, const Verifier& verifier, bool override)
    {
        UpgradePackHeaderPrologue headerPrologue;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(headerPrologue), address);
        address += sizeof(UpgradePackHeaderPrologue);

        bool sanity = (override || headerPrologue.status == UpgradePackStatus::readyToDeploy || headerPrologue.status == UpgradePackStatus::deployStarted) && headerPrologue.magic == upgradePackMagic;

        if (!sanity)
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
                if (TryLoadImage(ram, decryptor))
                    return true;

            MarkAsError(upgradeErrorCodeNoOrIncorrectSecondStageFound);
        }

        return false;
    }

    bool SecondStageToRamLoader::TryLoadImage(infra::ByteRange ram, Decryptor& decryptor)
    {
        ImageHeaderPrologue imageHeader;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(imageHeader), address);

        if (std::strncmp(imageHeader.targetName.data(), "boot2nd", ImageHeaderPrologue().targetName.size()) == 0)
        {
            address += sizeof(imageHeader);

            infra::ByteRange decryptorState = decryptor.StateBuffer();
            upgradePackFlash.ReadBuffer(decryptorState, address);
            decryptor.Reset();
            address += decryptorState.size();

            ImageHeaderEpilogue imageHeaderEpilogue;
            upgradePackFlash.ReadBuffer(infra::MakeByteRange(imageHeaderEpilogue), address);
            address += sizeof(imageHeaderEpilogue);

            decryptor.DecryptPart(infra::MakeByteRange(imageHeaderEpilogue));

            uint32_t imageSize = imageHeader.lengthOfHeaderAndImage - sizeof(imageHeader) - decryptorState.size() - sizeof(imageHeaderEpilogue);
            if (imageSize > ram.size())
                return false;

            ram.shrink_from_back_to(imageSize);
            upgradePackFlash.ReadBuffer(ram, address);

            return decryptor.DecryptAndAuthenticate(ram);
        }

        address += imageHeader.lengthOfHeaderAndImage;
        return false;
    }

    void SecondStageToRamLoader::MarkAsError(uint32_t errorCode)
    {
        static const UpgradePackStatus statusError = UpgradePackStatus::invalid;
        upgradePackFlash.WriteBuffer(infra::MakeByteRange(statusError), 0);
        upgradePackFlash.WriteBuffer(infra::MakeByteRange(errorCode), 4);
    }
}
