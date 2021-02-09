#ifndef UPGRADE_PACK_BUILD_LIBRARY_BUILD_UPGRADE_PACK_HPP
#define UPGRADE_PACK_BUILD_LIBRARY_BUILD_UPGRADE_PACK_HPP

#include "infra/syntax/Json.hpp"
#include "infra/util/ByteRange.hpp"
#include "upgrade/pack_builder/UpgradePackBuilder.hpp"
#include "upgrade/pack_builder/UpgradePackInputFactory.hpp"
#include <vector>
#include <utility>

namespace application
{
    using TargetAndFiles = std::vector<std::pair<std::string, std::string>>;
    using BuildOptions = std::vector<std::pair<std::string, std::string>>;

    int BuildUpgradePack(const application::UpgradePackBuilder::HeaderInfo& headerInfo, const std::vector<std::string>& supportedHexTargets,
        const std::vector<std::pair<std::string, uint32_t>>& supportedElfTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets,
        const std::vector<std::string>& mandatoryTargets, std::string outputFilename,
        TargetAndFiles targetAndFiles, BuildOptions buildOptions, infra::JsonObject& configuration, infra::ConstByteRange aesKey,
        infra::ConstByteRange ecDsa224PublicKey, infra::ConstByteRange ecDsa224PrivateKey, const std::vector<NoFileInputFactory*>& otherTargets = std::vector<NoFileInputFactory*>());

    class UpgradePackBuilderFacade
    {
    public:
        explicit UpgradePackBuilderFacade(const application::UpgradePackBuilder::HeaderInfo& headerInfo);
        virtual ~UpgradePackBuilderFacade() = default;

        void Build(const std::vector<std::string>& supportedHexTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedElfTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets,
            const std::vector<std::string>& mandatoryTargets, std::string outputFilename, TargetAndFiles& targetAndFiles, BuildOptions& buildOptions, infra::JsonObject& configuration,
            infra::ConstByteRange aesKey, infra::ConstByteRange ecDsa224PublicKey, infra::ConstByteRange ecDsa224PrivateKey, const std::vector<NoFileInputFactory*>& otherTargets);

        int Result() const;

    protected:
        virtual void PreBuilder(TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration);
        virtual void PostBuilder(UpgradePackBuilder& builder, ImageSigner& signer, const BuildOptions& buildOptions);

    private:
        void TryBuild(const std::vector<std::string>& supportedHexTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedElfTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets,
            const std::vector<std::string>& mandatoryTargets, std::string outputFilename, TargetAndFiles& targetAndFiles, BuildOptions& buildOptions, infra::JsonObject& configuration, infra::ConstByteRange aesKey, infra::ConstByteRange ecDsa224PublicKey,
            infra::ConstByteRange ecDsa224PrivateKey, const std::vector<NoFileInputFactory*>& otherTargets);

        void ShowUsage(int argc, const char* argv[], const std::vector<std::string>& supportedHexTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedElfTargets,
            const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets, const std::vector<NoFileInputFactory*>& otherTargets) const;
        void ShowUsage(TargetAndFiles& targetAndFiles, BuildOptions& buildOptions,
            const std::vector<std::string>& supportedHexTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedElfTargets,
            const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets,
            const std::vector<NoFileInputFactory*>& otherTargets) const;


    protected:
        UpgradePackBuilder::HeaderInfo headerInfo;                                                                      //TICS !INT#002

    private:
        int result = 0;
    };
}

#endif
