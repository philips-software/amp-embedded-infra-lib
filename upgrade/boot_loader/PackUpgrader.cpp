#include "upgrade/boot_loader/PackUpgrader.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include <cstring>

namespace application
{
    PackUpgrader::PackUpgrader(hal::SynchronousFlash& upgradePackFlash)
        : upgradePackFlash(upgradePackFlash)
    {
        services::GlobalTracer().Trace() << "PackUpgrader:: ctor";
    }

    void PackUpgrader::UpgradeFromImages(infra::MemoryRange<ImageUpgrader*> imageUpgraders)
    {
        services::GlobalTracer().Trace() << "PackUpgrader:: UpgradeFromImages, imageUpgraders begin: " << reinterpret_cast<uintptr_t>(imageUpgraders.begin()) << ", end: " << reinterpret_cast<uintptr_t>(imageUpgraders.end());
        services::GlobalTracer().Trace() << "PackUpgrader:: UpgradeFromImages, address: " << address;

        UpgradePackHeaderPrologue headerPrologue;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(headerPrologue), address);
        address += sizeof(UpgradePackHeaderPrologue);

        services::GlobalTracer().Trace() << "PackUpgrader:: UpgradeFromImages, UpgradePackHeaderPrologue address: " << infra::hex << reinterpret_cast<uintptr_t>(&headerPrologue);

        services::GlobalTracer().Trace() << "errorCode: " << headerPrologue.errorCode;
        services::GlobalTracer().Trace() << "magic: " << headerPrologue.magic[0] << " " << headerPrologue.magic[1] << " " << headerPrologue.magic[2];
        services::GlobalTracer().Trace() << "signatureLength" << headerPrologue.signatureLength;
        services::GlobalTracer().Trace() << "signedContentsLength" << headerPrologue.signedContentsLength;
        services::GlobalTracer().Trace() << "signatureMethod" << headerPrologue.signatureMethod;

        bool sanity = headerPrologue.magic == upgradePackMagic;

        services::GlobalTracer().Trace() << "PackUpgrader:: UpgradeFromImages, sanity? " << sanity;

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

        services::GlobalTracer().Trace() << "PackUpgrader:: HasImage, number of images: " << headerEpilogue.numberOfImages;

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

        services::GlobalTracer().Trace() << "PackUpgrader:: IsImage? " << (std::strncmp(imageHeader.targetName.data(), imageName, imageHeader.targetName.size()) == 0);

        return std::strncmp(imageHeader.targetName.data(), imageName, imageHeader.targetName.size()) == 0;
    }

    void PackUpgrader::MarkAsDeployStarted()
    {
        services::GlobalTracer().Trace() << "PackUpgrader:: MarkAsDeployStarted";

        WriteStatus(UpgradePackStatus::deployStarted);
    }

    void PackUpgrader::MarkAsDeployed()
    {
        services::GlobalTracer().Trace() << "PackUpgrader:: MarkAsDeployed";

        WriteStatus(UpgradePackStatus::deployed);
    }

    void PackUpgrader::MarkAsError(uint32_t errorCode)
    {
        services::GlobalTracer().Trace() << "PackUpgrader:: MarkAsError";

        WriteStatus(UpgradePackStatus::invalid);
        WriteError(errorCode);
    }

    void PackUpgrader::WriteStatus(UpgradePackStatus status)
    {
        services::GlobalTracer().Trace() << "PackUpgrader:: WriteStatus: " << static_cast<uint8_t>(status);

        upgradePackFlash.WriteBuffer(infra::MakeByteRange(status), 0);
    }

    void PackUpgrader::WriteError(uint32_t errorCode)
    {
        services::GlobalTracer().Trace() << "PackUpgrader:: WriteError: " << errorCode;

        upgradePackFlash.WriteBuffer(infra::MakeByteRange(errorCode), 4);
    }
}
