#ifndef UPGRADE_BUILD_UPGRADE_PACK_HPP
#define UPGRADE_BUILD_UPGRADE_PACK_HPP

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

    int BuildUpgradePack(const application::UpgradePackBuilder::HeaderInfo& headerInfo, const SupportedTargets& targets, const std::string& outputFilename,
        const TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration, infra::ConstByteRange aesKey,
        infra::ConstByteRange ecDsa224PublicKey, infra::ConstByteRange ecDsa224PrivateKey);

    class UpgradePackBuilderFacade
    {
    public:
        explicit UpgradePackBuilderFacade(const application::UpgradePackBuilder::HeaderInfo& headerInfo);
        virtual ~UpgradePackBuilderFacade() = default;

        void Build(const SupportedTargets& targets, const std::string& outputFilename, const TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration,
            infra::ConstByteRange aesKey, infra::ConstByteRange ecDsa224PublicKey, infra::ConstByteRange ecDsa224PrivateKey);

        int Result() const;

    protected:
        virtual void PreBuilder(const TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration);
        virtual void PostBuilder(UpgradePackBuilder& builder, ImageSigner& signer, const BuildOptions& buildOptions);

    private:
        void TryBuild(const SupportedTargets& targets, const std::string& outputFilename, const TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration,
            infra::ConstByteRange aesKey, infra::ConstByteRange ecDsa224PublicKey, infra::ConstByteRange ecDsa224PrivateKey);

        void ShowUsage(int argc, const char* argv[], const SupportedTargets& targets) const;
        void ShowUsage(const TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, const SupportedTargets& targets) const;

    protected:
        UpgradePackBuilder::HeaderInfo headerInfo;

    private:
        int result = 0;
    };
}

#endif
