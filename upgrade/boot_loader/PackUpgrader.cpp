#include "upgrade/boot_loader/PackUpgrader.hpp"
#include <cstring>

namespace application
{
    PackUpgrader::PackUpgrader(hal::SynchronousFlash& upgradePackFlash)
        : upgradePackFlash(upgradePackFlash)
    {}

    void PackUpgrader::UpgradeFromImages(infra::MemoryRange<ImageUpgrader*> imageUpgraders)
    {
        UpgradePackHeaderPrologue headerPrologue;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(headerPrologue), address);
        address += sizeof(UpgradePackHeaderPrologue);

        bool sanity = headerPrologue.magic == upgradePackMagic;
        if (!sanity)
            return;

        MarkAsDeployStarted();

        address += headerPrologue.signatureLength;

        UpgradePackHeaderEpilogue headerEpilogue;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(headerEpilogue), address);
        address += sizeof(UpgradePackHeaderEpilogue);

        if (headerEpilogue.headerVersion != 1)
            MarkAsError(upgradeErrorCodeUnknownHeaderVersion);
        else
        {
            for (std::size_t imageIndex = 0; imageIndex != headerEpilogue.numberOfImages; ++imageIndex)
            {
                if (!TryUpgradeImage(imageUpgraders))
                    return;
            }

            MarkAsDeployed();
        }
    }

    bool PackUpgrader::HasImage(const char* imageName) const
    {
        uint32_t imageAddress = 0;

        UpgradePackHeaderPrologue headerPrologue;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(headerPrologue), imageAddress);
        imageAddress += sizeof(UpgradePackHeaderPrologue);
        imageAddress += headerPrologue.signatureLength;

        UpgradePackHeaderEpilogue headerEpilogue;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(headerEpilogue), imageAddress);
        imageAddress += sizeof(UpgradePackHeaderEpilogue);

        for (std::size_t imageIndex = 0; imageIndex != headerEpilogue.numberOfImages; ++imageIndex)
        {
            if (IsImage(imageAddress, imageName))
                return true;
        }

        return false;
    }

    bool PackUpgrader::TryUpgradeImage(infra::MemoryRange<ImageUpgrader*> imageUpgraders)
    {
        ImageHeaderPrologue imageHeader;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(imageHeader), address);

        uint32_t imageAddress = address;
        address += imageHeader.lengthOfHeaderAndImage;

        for (auto& imageUpgrader : imageUpgraders)
        {
            if (std::strncmp(imageHeader.targetName.data(), imageUpgrader->TargetName(), imageHeader.targetName.size()) == 0)
            {
                imageAddress += sizeof(imageHeader);

                infra::ByteRange decryptorState = imageUpgrader->ImageDecryptor().StateBuffer();
                upgradePackFlash.ReadBuffer(decryptorState, imageAddress);
                imageUpgrader->ImageDecryptor().Reset();
                imageAddress += decryptorState.size();

                ImageHeaderEpilogue imageHeaderEpilogue;
                upgradePackFlash.ReadBuffer(infra::MakeByteRange(imageHeaderEpilogue), imageAddress);
                imageAddress += sizeof(imageHeaderEpilogue);

                imageUpgrader->ImageDecryptor().DecryptPart(infra::MakeByteRange(imageHeaderEpilogue));
                uint32_t upgradeResult = imageUpgrader->Upgrade(upgradePackFlash, imageAddress, imageHeaderEpilogue.imageSize, imageHeaderEpilogue.destinationAddress);
                if (upgradeResult != 0)
                {
                    MarkAsError(upgradeResult);
                    return false;
                }

                return true;
            }
        }

        MarkAsError(upgradeErrorCodeNoSuitableImageUpgraderFound);
        return false;
    }

    bool PackUpgrader::IsImage(uint32_t& imageAddress, const char* imageName) const
    {
        ImageHeaderPrologue imageHeader;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(imageHeader), imageAddress);
        imageAddress += imageHeader.lengthOfHeaderAndImage;

        return std::strncmp(imageHeader.targetName.data(), imageName, imageHeader.targetName.size()) == 0;
    }

    void PackUpgrader::MarkAsDeployStarted()
    {
        WriteStatus(UpgradePackStatus::deployStarted);
    }

    void PackUpgrader::MarkAsDeployed()
    {
        WriteStatus(UpgradePackStatus::deployed);
    }

    void PackUpgrader::MarkAsError(uint32_t errorCode)
    {
        WriteStatus(UpgradePackStatus::invalid);
        WriteError(errorCode);
    }

    void PackUpgrader::WriteStatus(UpgradePackStatus status)
    {
        upgradePackFlash.WriteBuffer(infra::MakeByteRange(status), 0);
    }

    void PackUpgrader::WriteError(uint32_t errorCode)
    {
        upgradePackFlash.WriteBuffer(infra::MakeByteRange(errorCode), 4);
    }
}
