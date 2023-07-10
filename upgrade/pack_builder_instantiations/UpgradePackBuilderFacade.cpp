#include "upgrade/pack_builder_instantiations/UpgradePackBuilderFacade.hpp"
#include "hal/generic/FileSystemGeneric.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "upgrade/pack_builder/BinaryObject.hpp"
#include "upgrade/pack_builder/ImageEncryptorAes.hpp"
#include "upgrade/pack_builder/ImageEncryptorNone.hpp"
#include "upgrade/pack_builder/ImageSignerEcDsa.hpp"
#include "upgrade/pack_builder/ImageSignerHashOnly.hpp"
#include "upgrade/pack_builder/Input.hpp"
#include "upgrade/pack_builder/UpgradePackBuilder.hpp"
#include "upgrade/pack_builder/UpgradePackInputFactory.hpp"

namespace main_
{
    struct MissingTargetException
        : std::runtime_error
    {
        explicit MissingTargetException(const std::string& target)
            : std::runtime_error("Missing mandatory target: " + target)
        {}
    };

    struct IncorrectOrderOfTargetException
        : std::runtime_error
    {
        explicit IncorrectOrderOfTargetException(const std::string& target)
            : std::runtime_error("Incorrect order of target: " + target)
        {}
    };

    UpgradePackBuilderFacade::UpgradePackBuilderFacade(const application::UpgradePackBuilder::HeaderInfo& headerInfo)
        : headerInfo(headerInfo)
    {}

    void UpgradePackBuilderFacade::Build(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename,
        const BuildOptions& buildOptions, infra::JsonObject& configuration, const DefaultKeyMaterial& keys)
    {
        hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
        hal::FileSystemGeneric fileSystem;
        application::ImageEncryptorAes encryptor(randomDataGenerator, keys.aesKey);
        application::UpgradePackInputFactory inputFactory(fileSystem, supportedTargets, encryptor);
        application::ImageSignerEcDsa signer(randomDataGenerator, keys.ecDsa224PublicKey, keys.ecDsa224PrivateKey);

        PreBuilder(requestedTargets, buildOptions, configuration);
        application::UpgradePackBuilder builder(headerInfo, std::move(CreateInputs(supportedTargets, requestedTargets, inputFactory)), signer);
        PostBuilder(builder, signer, buildOptions);

        builder.WriteUpgradePack(outputFilename, fileSystem);
    }

    void UpgradePackBuilderFacade::Build(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename)
    {
        hal::FileSystemGeneric fileSystem;
        application::ImageEncryptorNone encryptor;
        application::UpgradePackInputFactory inputFactory(fileSystem, supportedTargets, encryptor);
        application::ImageSignerHashOnly signer;
        application::UpgradePackBuilder builder(headerInfo, std::move(CreateInputs(supportedTargets, requestedTargets, inputFactory)), signer);

        builder.WriteUpgradePack(outputFilename, fileSystem);
    }

    infra::Optional<uint8_t> UpgradePackBuilderFacade::GetOrder(const std::string& targetName, const std::map<uint8_t, std::vector<std::string>>& orderedTargets) const
    {
        infra::Optional<uint8_t> order(infra::inPlace, 0);
        for (const auto& targets : orderedTargets)
        {
            ++(*order);
            const auto targetPos = std::find(targets.second.begin(), targets.second.end(), targetName);

            if (targetPos != targets.second.end())
                return order;
        }
        return infra::none;
    }

    bool UpgradePackBuilderFacade::CheckIfTargetIsInOrder(const std::string& target, const application::SupportedTargets& supportedTargets)
    {
        static uint8_t currentOrderOfTarget = 1;
        static const auto& orderedTargets = supportedTargets.OrderOfTargets();
        const auto orderToAdd = GetOrder(target, orderedTargets);
        if (orderToAdd)
        {
            if (currentOrderOfTarget > *orderToAdd)
                return false;
            else
                currentOrderOfTarget = *orderToAdd;
        }
        return true;
    }

    std::vector<std::unique_ptr<application::Input>> UpgradePackBuilderFacade::CreateInputs(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, application::InputFactory& factory)
    {
        std::vector<std::unique_ptr<application::Input>> inputs;

        for (const auto& [target, file, address] : requestedTargets)
        {
            if (!CheckIfTargetIsInOrder(target, supportedTargets))
                throw IncorrectOrderOfTargetException(target);
            inputs.push_back(factory.CreateInput(target, file, address));
        }

        for (const auto& mandatoryTarget : supportedTargets.MandatoryTargets())
        {
            bool found = false;
            for (const auto& input : inputs)
                if (input->TargetName() == mandatoryTarget)
                    found = true;

            if (!found)
                throw MissingTargetException(mandatoryTarget);
        }

        return inputs;
    }

    void UpgradePackBuilderFacade::PreBuilder(const TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration)
    {}

    void UpgradePackBuilderFacade::PostBuilder(application::UpgradePackBuilder& builder, application::ImageSigner& signer, const BuildOptions& buildOptions)
    {}
}
