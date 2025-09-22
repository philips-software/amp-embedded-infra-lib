#include "upgrade/boot_loader/UpgradePackLoader.hpp"
#include "infra/util/Compatibility.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"
#include <cstring>

namespace application
{
    UpgradePackLoader::UpgradePackLoader(hal::SynchronousFlash& upgradePackFlash, const char* product)
        : upgradePackFlash(upgradePackFlash)
        , product(product)
    {
        services::GlobalTracer().Trace() << "UpgradePackLoader:: constructed.";
    }

    bool UpgradePackLoader::Load(Decryptor& decryptor, const Verifier& verifier)
    {
        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load start";

        UpgradePackHeaderPrologue headerPrologue;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(headerPrologue), address);
        headerPrologue.status = ReadStatus();
        address += sizeof(UpgradePackHeaderPrologue);

        bool isSane = (headerPrologue.status == UpgradePackStatus::readyToDeploy ||
                          headerPrologue.status == UpgradePackStatus::deployStarted) &&
                      headerPrologue.magic == upgradePackMagic;

        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - IsSane? " << isSane;

        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerPrologue.status " << headerPrologue.status;
        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerPrologue.errorCode " << headerPrologue.errorCode;
        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerPrologue.magic " << headerPrologue.magic[0] << " " << headerPrologue.magic[1] << " " << headerPrologue.magic[2];
        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerPrologue.signatureLength " << headerPrologue.signatureLength;
        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerPrologue.signatureMethod " << headerPrologue.signatureMethod;
        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerPrologue.signedContentsLength " << headerPrologue.signedContentsLength;

        if (!isSane)
            return false;

        hal::SynchronousFlash::Range signature(address, address + headerPrologue.signatureLength);
        address += headerPrologue.signatureLength;

        hal::SynchronousFlash::Range signedContents(address, address + headerPrologue.signedContentsLength);

        UpgradePackHeaderEpilogue headerEpilogue;
        upgradePackFlash.ReadBuffer(infra::MakeByteRange(headerEpilogue), address);
        address += sizeof(UpgradePackHeaderEpilogue);

        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerEpilogue.headerVersion " << headerEpilogue.headerVersion;
        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerEpilogue.headerLength " << headerEpilogue.headerLength;
        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerEpilogue.numberOfImages " << headerEpilogue.numberOfImages;
        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerEpilogue.componentVersion " << headerEpilogue.componentVersion;

        std::size_t index = 0;

        while (headerEpilogue.productName[index] != '\0')
            services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerEpilogue.productName " << headerEpilogue.productName[index++];

        index = 0;
        while (headerEpilogue.productVersion[index] != '\0')
            services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerEpilogue.productName " << headerEpilogue.productVersion[index++];

        index = 0;
        while (headerEpilogue.componentName[index] != '\0')
            services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerEpilogue.productName " << headerEpilogue.componentName[index++];

        if (headerEpilogue.headerVersion != 1)
        {
            services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerEpilogue.headerVersion != 1: " << (headerEpilogue.headerVersion != 1);

            MarkAsError(upgradeErrorCodeUnknownHeaderVersion);
        }
        else if (std::strcmp(product, headerEpilogue.productName.data()) != 0)
        {
            services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - std::strcmp(product, headerEpilogue.productName.data()) != 0: " << (headerEpilogue.headerVersion != 1);

            MarkAsError(upgradeErrorCodeUnknownProductName);
        }
        else if (!verifier.IsValid(upgradePackFlash, signature, signedContents))
        {
            services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - !verifier.IsValid(upgradePackFlash, signature, signedContents): " << (!verifier.IsValid(upgradePackFlash, signature, signedContents));

            MarkAsError(upgradeErrorCodeInvalidSignature);
        }
        else
        {
            services::GlobalTracer().Trace() << "UpgradePackLoader:: Load - headerEpilogue.headerVersion != 1 " << (headerEpilogue.headerVersion != 1);

            return PostLoadActions(headerEpilogue.numberOfImages, decryptor);
        }

        services::GlobalTracer().Trace() << "UpgradePackLoader:: Load false";

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
