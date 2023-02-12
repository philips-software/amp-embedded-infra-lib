#include "upgrade/pack_builder/BuildUpgradePack.hpp"
#include "hal/generic/FileSystemGeneric.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "mbedtls/memory_buffer_alloc.h"
#include "upgrade/pack_builder/BinaryObject.hpp"
#include "upgrade/pack_builder/ImageEncryptorAes.hpp"
#include "upgrade/pack_builder/ImageEncryptorNone.hpp"
#include "upgrade/pack_builder/ImageSignerEcDsa.hpp"
#include "upgrade/pack_builder/ImageSignerHashOnly.hpp"
#include "upgrade/pack_builder/Input.hpp"
#include "upgrade/pack_builder/UpgradePackBuilder.hpp"
#include "upgrade/pack_builder/UpgradePackInputFactory.hpp"
#include <iostream>

namespace application
{
    namespace
    {
        struct MissingTargetException
            : std::runtime_error
        {
            explicit MissingTargetException(const std::string& target)
                : std::runtime_error("Missing mandatory target: " + target)
            {}
        };
    }

    void BuildUpgradePack(const application::UpgradePackBuilder::HeaderInfo& headerInfo, const SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets,
        const std::string& outputFilename, const BuildOptions& buildOptions, infra::JsonObject& configuration, const DefaultKeyMaterial& keys)
    {
        return UpgradePackBuilderFacade(headerInfo).Build(supportedTargets, requestedTargets, outputFilename, buildOptions, configuration, keys);
    }

    void BuildUpgradePack(const application::UpgradePackBuilder::HeaderInfo& headerInfo, const SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets,
        const std::string& outputFilename)
    {
        return UpgradePackBuilderFacade(headerInfo).Build(supportedTargets, requestedTargets, outputFilename);
    }

    UpgradePackBuilderFacade::UpgradePackBuilderFacade(const application::UpgradePackBuilder::HeaderInfo& headerInfo)
        : headerInfo(headerInfo)
    {
        // Initialize the MbedTLS memory pool
        unsigned char memory_buf[100000];
        mbedtls_memory_buffer_alloc_init(memory_buf, sizeof(memory_buf));
    }

    void UpgradePackBuilderFacade::Build(const SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename,
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

    void UpgradePackBuilderFacade::Build(const SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, const std::string& outputFilename)
    {
        hal::FileSystemGeneric fileSystem;
        application::ImageEncryptorNone encryptor;
        application::UpgradePackInputFactory inputFactory(fileSystem, supportedTargets, encryptor);
        application::ImageSignerHashOnly signer;
        application::UpgradePackBuilder builder(headerInfo, std::move(CreateInputs(supportedTargets, requestedTargets, inputFactory)), signer);

        builder.WriteUpgradePack(outputFilename, fileSystem);
    }

    std::vector<std::unique_ptr<application::Input>> UpgradePackBuilderFacade::CreateInputs(const SupportedTargets& supportedTargets, const TargetAndFiles& requestedTargets, InputFactory& factory)
    {
        std::vector<std::unique_ptr<application::Input>> inputs;

        for (const auto& [target, file, address] : requestedTargets)
            inputs.push_back(factory.CreateInput(target, file, address));

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

    void UpgradePackBuilderFacade::PostBuilder(UpgradePackBuilder& builder, ImageSigner& signer, const BuildOptions& buildOptions)
    {}
}
