#ifndef UPGRADE_BUILD_REFERENCE_UPGRADE_PACK_HPP
#define UPGRADE_BUILD_REFERENCE_UPGRADE_PACK_HPP

#include "infra/util/ByteRange.hpp"
#include "upgrade/pack_builder/BuildUpgradePack.hpp"
#include <utility>
#include <vector>

namespace application
{
    int BuildReferenceUpgradePack(const application::UpgradePackBuilder::HeaderInfo& headerInfo, const std::vector<std::string>& supportedHexTargets,
        const std::vector<std::pair<std::string, uint32_t>>& supportedElfTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets, std::string outputFilename,
        std::vector<std::pair<std::string, std::string>>& targetAndFiles, std::vector<std::pair<std::string, std::string>>& buildOptions, infra::JsonObject& configuration, infra::ConstByteRange aesKey,
        infra::ConstByteRange ecDsa224PublicKey, infra::ConstByteRange ecDsa224PrivateKey, const std::vector<NoFileInputFactory*>& otherTargets = std::vector<NoFileInputFactory*>());

    class ReferenceUpgradePackBuilderFacade
        : public UpgradePackBuilderFacade
    {
    public:
        explicit ReferenceUpgradePackBuilderFacade(const application::UpgradePackBuilder::HeaderInfo& headerInfo);

    protected:
        virtual void PreBuilder(std::vector<std::pair<std::string, std::string>>& targetAndFiles, const std::vector<std::pair<std::string, std::string>>& buildOptions, infra::JsonObject& configuration) override;
        virtual void PostBuilder(UpgradePackBuilder& builder, ImageSigner& signer, const std::vector<std::pair<std::string, std::string>>& buildOptions) override;
    };
}

#endif
