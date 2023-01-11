#ifndef UPGRADE_BUILD_UPGRADE_PACK_HPP
#define UPGRADE_BUILD_UPGRADE_PACK_HPP

#include "infra/syntax/Json.hpp"
#include "infra/util/ByteRange.hpp"
#include "upgrade/pack_builder/UpgradePackBuilder.hpp"
#include "upgrade/pack_builder/UpgradePackInputFactory.hpp"
#include <utility>
#include <vector>

namespace application
{
    using TargetAndFiles = std::vector<std::tuple<std::string, std::string, infra::Optional<uint32_t>>>;
    using BuildOptions = std::vector<std::pair<std::string, std::string>>;

    struct DefaultKeyMaterial
    {
        infra::ConstByteRange aesKey;
        infra::ConstByteRange ecDsa224PublicKey;
        infra::ConstByteRange ecDsa224PrivateKey;
    };

    void BuildUpgradePack(const application::UpgradePackBuilder::HeaderInfo& headerInfo, const SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets,
        const std::string& outputFilename, const BuildOptions& buildOptions, infra::JsonObject& configuration, const DefaultKeyMaterial& keys);

    void BuildUpgradePack(const application::UpgradePackBuilder::HeaderInfo& headerInfo, const SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets,
        const std::string& outputFilename);

    class UpgradePackBuilderFacade
    {
    public:
        explicit UpgradePackBuilderFacade(const application::UpgradePackBuilder::HeaderInfo& headerInfo);
        virtual ~UpgradePackBuilderFacade() = default;

        void Build(const SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename,
            const BuildOptions& buildOptions, infra::JsonObject& configuration, const DefaultKeyMaterial& keys);

        void Build(const SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename);

    protected:
        virtual void PreBuilder(const TargetAndFiles& requestedTargets, const BuildOptions& buildOptions, infra::JsonObject& configuration);
        virtual void PostBuilder(UpgradePackBuilder& builder, ImageSigner& signer, const BuildOptions& buildOptions);

    private:
        std::vector<std::unique_ptr<application::Input>> CreateInputs(const SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, InputFactory& factory);

    protected:
        UpgradePackBuilder::HeaderInfo headerInfo;
    };
}

#endif
