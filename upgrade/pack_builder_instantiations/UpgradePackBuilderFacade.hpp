#ifndef UPGRADE_UPGRADE_PACK_BUILDER_FACADE_HPP
#define UPGRADE_UPGRADE_PACK_BUILDER_FACADE_HPP

#include "infra/syntax/Json.hpp"
#include "infra/util/ByteRange.hpp"
#include "upgrade/pack_builder/UpgradePackBuilder.hpp"
#include "upgrade/pack_builder/UpgradePackInputFactory.hpp"
#include <utility>
#include <vector>

namespace main_
{
    using TargetAndFiles = std::vector<std::tuple<std::string, std::string, infra::Optional<uint32_t>>>;
    using BuildOptions = std::vector<std::pair<std::string, std::string>>;

    struct DefaultKeyMaterial
    {
        infra::ConstByteRange aesKey;
        infra::ConstByteRange ecDsa224PublicKey;
        infra::ConstByteRange ecDsa224PrivateKey;
    };

    class UpgradePackBuilderFacade
    {
    public:
        explicit UpgradePackBuilderFacade(const application::UpgradePackBuilder::HeaderInfo& headerInfo);
        virtual ~UpgradePackBuilderFacade() = default;

        void Build(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename,
            const BuildOptions& buildOptions, infra::JsonObject& configuration, const DefaultKeyMaterial& keys);

        void Build(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename);

    protected:
        virtual void PreBuilder(const TargetAndFiles& requestedTargets, const BuildOptions& buildOptions, infra::JsonObject& configuration);
        virtual void PostBuilder(application::UpgradePackBuilder& builder, application::ImageSigner& signer, const BuildOptions& buildOptions);

    private:
        infra::Optional<uint8_t> GetOrder(const std::string& targetName, const std::vector<std::vector<std::string>>& orderedTargets) const;
        bool CheckIfTargetIsInOrder(const std::string& target, const application::SupportedTargets& supportedTargets);
        std::vector<std::unique_ptr<application::Input>> CreateInputs(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, application::InputFactory& factory);

    protected:
        application::UpgradePackBuilder::HeaderInfo headerInfo;
    };
}

#endif
