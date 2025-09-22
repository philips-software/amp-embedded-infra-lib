#include "upgrade/boot_loader/SecondStageToRamLoader.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"
#include <cstring>

namespace application
{
    SecondStageToRamLoader::SecondStageToRamLoader(hal::SynchronousFlash& upgradePackFlash, const char* product, infra::ByteRange ram)
        : UpgradePackLoader(upgradePackFlash, product)
        , ram(ram)
    {
        services::GlobalTracer().Trace() << "SecondStageToRamLoader:: constructed.";
    }

    bool SecondStageToRamLoader::PostLoadActions(uint32_t numberOfImages, Decryptor& decryptor)
    {
        services::GlobalTracer().Trace() << "SecondStageToRamLoader:: PostLoadActions.";

        for (uint32_t imageIndex = 0; imageIndex != numberOfImages; ++imageIndex)
        {
            services::GlobalTracer().Trace() << "SecondStageToRamLoader:: image index: " << imageIndex;

            ImageHeaderPrologue imageHeader;
            upgradePackFlash.ReadBuffer(infra::MakeByteRange(imageHeader), address);

            if (std::strncmp(imageHeader.targetName.data(), "boot2nd", ImageHeaderPrologue().targetName.size()) == 0)
            {
                services::GlobalTracer().Trace() << "SecondStageToRamLoader:: does contain boot2nd? " << (std::strncmp(imageHeader.targetName.data(), "boot2nd", ImageHeaderPrologue().targetName.size()) == 0);

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

                services::GlobalTracer().Trace() << "SecondStageToRamLoader:: is imageSize bigger than RAM? " << (imageSize > ram.size());

                if (imageSize > ram.size())
                    break;

                ram.shrink_from_back_to(imageSize);

                upgradePackFlash.ReadBuffer(ram, address);

                services::GlobalTracer().Trace() << "SecondStageToRamLoader:: read buffer, address: 0x" << infra::hex << address;

                if (decryptor.DecryptAndAuthenticate(ram))
                {
                    services::GlobalTracer().Trace() << "SecondStageToRamLoader:: DecryptAndAuthenticate? TRUE";
                    return true;
                }
                else
                {
                    services::GlobalTracer().Trace() << "SecondStageToRamLoader:: DecryptAndAuthenticate? FALSE";
                    break;
                }
            }

            address += imageHeader.lengthOfHeaderAndImage;

            services::GlobalTracer().Trace() << "SecondStageToRamLoader:: increment address " << address;
        }

        services::GlobalTracer().Trace() << "SecondStageToRamLoader:: mark as error  upgradeErrorCodeNoOrIncorrectSecondStageFound";

        MarkAsError(upgradeErrorCodeNoOrIncorrectSecondStageFound);

        return false;
    }
}
