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
            : std::runtime_error("Incorrect order of target : " + target)
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

    uint8_t UpgradePackBuilderFacade::GetOrder(const std::string& targetName, const std::vector<std::vector<std::string>>& orderedTargets)
    {
        uint8_t order = 0;
        for (auto targets : orderedTargets)
        {
            ++order;
            auto targetPos = std::find(targets.begin(), targets.end(), targetName);

            if (targetPos != targets.end())
                return order;
        }
        return 0;
    }

    std::vector<std::unique_ptr<application::Input>> UpgradePackBuilderFacade::CreateInputs(const application::SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, application::InputFactory& factory)
    {
        std::vector<std::unique_ptr<application::Input>> inputs;

        uint8_t currentOrderOfTarget = 1;
        auto orderedTargets = supportedTargets.OrderOfTargets();

        for (const auto& [target, file, address] : requestedTargets)
        {
            auto orderToAdd = GetOrder(target, orderedTargets);
            if (orderToAdd != 0)
            {
                if (currentOrderOfTarget > orderToAdd)
                    throw IncorrectOrderOfTargetException(target);
                else
                    currentOrderOfTarget = orderToAdd;
            }
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
