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

    struct DefaultKey224Material
    {
        infra::ConstByteRange aesKey;
        infra::ConstByteRange ecDsa224PublicKey;
        infra::ConstByteRange ecDsa224PrivateKey;
    };

    struct DefaultKey256Material
    {
        infra::ConstByteRange aesKey;
        infra::ConstByteRange ecDsa256PublicKey;
        infra::ConstByteRange ecDsa256PrivateKey;
    };

    class UpgradePackBuilderFacade
    {
    public:
        explicit UpgradePackBuilderFacade(const application::UpgradePackBuilder::HeaderInfo& headerInfo);
        virtual ~UpgradePackBuilderFacade() = default;

        void Build(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename,
            const BuildOptions& buildOptions, infra::JsonObject& configuration, const DefaultKey224Material& keys);

        void Build(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename,
            const BuildOptions& buildOptions, infra::JsonObject& configuration, const DefaultKey256Material& keys);

        void Build(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename);

    protected:
        virtual void PreBuilder(const TargetAndFiles& requestedTargets, const BuildOptions& buildOptions, infra::JsonObject& configuration);
        virtual void PostBuilder(application::UpgradePackBuilder& builder, application::ImageSigner& signer, const BuildOptions& buildOptions);

    private:
        void Build(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename,
            const BuildOptions& buildOptions, infra::JsonObject& configuration, const application::ImageSecurity& encryptor, application::ImageSigner& signer);
        infra::Optional<uint8_t> GetOrder(const std::string& targetName, const std::map<uint8_t, std::vector<std::string>>& orderedTargets) const;
        bool IsTargetInOrder(const std::string& target, const std::map<uint8_t, std::vector<std::string>>& orderedTargets) const;
        void UpdateCurrentOrderOfTarget(const std::string& target, const std::map<uint8_t, std::vector<std::string>>& orderedTargets);
        std::vector<std::unique_ptr<application::Input>> CreateInputs(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, application::InputFactory& factory);

    private:
        uint8_t currentOrderOfTarget = 0;

    protected:
        application::UpgradePackBuilder::HeaderInfo headerInfo;
    };
}

#endif
