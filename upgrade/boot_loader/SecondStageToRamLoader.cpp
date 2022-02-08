#include "upgrade/boot_loader/SecondStageToRamLoader.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"
#include <cstring>

namespace application
{
    SecondStageToRamLoader::SecondStageToRamLoader(hal::SynchronousFlash& upgradePackFlash, const char* product, infra::ByteRange ram)
        : UpgradePackLoader(upgradePackFlash, product)
        , ram(ram)
    {}

    bool SecondStageToRamLoader::Load(Decryptor& decryptor, const Verifier& verifier)
    {
        if (UpgradePackLoader::Load(decryptor, verifier))
            return true;
        else
            MarkAsError(upgradeErrorCodeNoOrIncorrectSecondStageFound);

        return false;
    }

    bool SecondStageToRamLoader::ImageFound(Decryptor& decryptor)
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
}
