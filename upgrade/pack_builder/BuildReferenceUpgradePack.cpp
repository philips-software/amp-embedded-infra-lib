#include "upgrade/pack_builder/BuildReferenceUpgradePack.hpp"
#include "upgrade/pack_builder/UpgradePackBuilder.hpp"
#include "upgrade/pack_builder/UpgradePackInputFactory.hpp"
#include <cctype>

namespace application
{
    namespace
    {
        std::string ToLower(const std::string& str)
        {
            std::string result;
			std::transform(str.begin(), str.end(), std::back_inserter(result), [](unsigned char c) { return std::tolower(c); });
            return result;
        }

        struct UsageException
        {};
    }

    int BuildReferenceUpgradePack(const application::UpgradePackBuilder::HeaderInfo& headerInfo, const std::vector<std::string>& supportedHexTargets,
        const std::vector<std::pair<std::string, uint32_t>>& supportedElfTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets, std::string outputFilename,
        TargetAndFiles& targetAndFiles, BuildOptions& buildOptions, infra::JsonObject& configuration, infra::ConstByteRange aesKey,
        infra::ConstByteRange ecDsa224PublicKey, infra::ConstByteRange ecDsa224PrivateKey, const std::vector<NoFileInputFactory*>& otherTargets)
    {
        ReferenceUpgradePackBuilderFacade builderFacade(headerInfo);
        builderFacade.Build(supportedHexTargets, supportedElfTargets, supportedBinaryTargets, outputFilename, targetAndFiles, buildOptions, configuration, aesKey, ecDsa224PublicKey, ecDsa224PrivateKey, otherTargets);
        return builderFacade.Result();
    }

    ReferenceUpgradePackBuilderFacade::ReferenceUpgradePackBuilderFacade(const application::UpgradePackBuilder::HeaderInfo& headerInfo)
        : UpgradePackBuilderFacade(headerInfo)
    {}

    void ReferenceUpgradePackBuilderFacade::PreBuilder(TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration)
    {
        for (auto option : buildOptions)
        {
            if (option.first == "invalidProduct")
                headerInfo.productName = "Unknown Product Name";
        }
    }

    void ReferenceUpgradePackBuilderFacade::PostBuilder(UpgradePackBuilder& builder, ImageSigner& signer, const BuildOptions& buildOptions)
    {
        for (auto option : buildOptions)
        {
            if (option.first == "invalidHeaderVersion")
                reinterpret_cast<UpgradePackHeaderEpilogue*>(builder.UpgradePack().data() + sizeof(UpgradePackHeaderPrologue) + signer.SignatureLength())->headerVersion = 0xff;
            else if (option.first == "invalidSignature")
                std::fill(builder.UpgradePack().begin() + sizeof(UpgradePackHeaderPrologue), builder.UpgradePack().begin() + sizeof(UpgradePackHeaderPrologue) + signer.SignatureLength(), 0xff);
        }
    }
}
