#include "mbedtls/memory_buffer_alloc.h"
#include "hal/generic/FileSystemGeneric.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "upgrade/pack_builder/BinaryObject.hpp"
#include "upgrade/pack_builder/BuildUpgradePack.hpp"
#include "upgrade/pack_builder/ImageEncryptorAes.hpp"
#include "upgrade/pack_builder/ImageSignerEcDsa.hpp"
#include "upgrade/pack_builder/Input.hpp"
#include "upgrade/pack_builder/UpgradePackBuilder.hpp"
#include "upgrade/pack_builder/UpgradePackInputFactory.hpp"
#include <iostream>

namespace application
{
    namespace
    {
        struct UsageException
        {};

        struct MissingTargetException
            : std::runtime_error
        {
            MissingTargetException(const std::string& target)
                : std::runtime_error("Missing target: " + target)
            {}
        };

        std::ostream& operator<<(std::ostream& stream, const application::SupportedTargets& targets)
        {
            for (const auto& target : targets.CmdTargets())
                stream << target << " ";
            for (const auto& target : targets.HexTargets())
                stream << target << " ";
            for (const auto& target : targets.ElfTargets())
                stream << target.first << " ";
            for (const auto& target : targets.BinTargets())
                stream << target.first << " ";

            return stream;
        }
    }

    int BuildUpgradePack(const application::UpgradePackBuilder::HeaderInfo& headerInfo, const SupportedTargets& targets, const std::string& outputFilename,
        const TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration, infra::ConstByteRange aesKey,
        infra::ConstByteRange ecDsa224PublicKey, infra::ConstByteRange ecDsa224PrivateKey)
    {
        UpgradePackBuilderFacade builderFacade(headerInfo);
        builderFacade.Build(targets, outputFilename, targetAndFiles, buildOptions, configuration, aesKey, ecDsa224PublicKey, ecDsa224PrivateKey);
        return builderFacade.Result();
    }

    UpgradePackBuilderFacade::UpgradePackBuilderFacade(const application::UpgradePackBuilder::HeaderInfo& headerInfo)
        : headerInfo(headerInfo)
    {
        // Initialize the MbedTLS memory pool
        unsigned char memory_buf[100000];
        mbedtls_memory_buffer_alloc_init(memory_buf, sizeof(memory_buf));
    }

    void UpgradePackBuilderFacade::Build(const SupportedTargets& targets, const std::string& outputFilename, const TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration, infra::ConstByteRange aesKey, infra::ConstByteRange ecDsa224PublicKey,
        infra::ConstByteRange ecDsa224PrivateKey)
    {
        try
        {
            TryBuild(targets, outputFilename, targetAndFiles, buildOptions, configuration, aesKey, ecDsa224PublicKey, ecDsa224PrivateKey);
        }
        catch (UsageException&)
        {
            ShowUsage(targetAndFiles, buildOptions, targets);
            result = 1;
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
            result = 1;
        }
    }

    void UpgradePackBuilderFacade::TryBuild(const SupportedTargets& targets, const std::string& outputFilename, const TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration, infra::ConstByteRange aesKey, infra::ConstByteRange ecDsa224PublicKey,
        infra::ConstByteRange ecDsa224PrivateKey)
    {
        hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
        hal::FileSystemGeneric fileSystem;
        application::ImageEncryptorAes imageEncryptorAes(randomDataGenerator, aesKey);
        application::UpgradePackInputFactory inputFactory(fileSystem, targets, imageEncryptorAes);
        application::ImageSignerEcDsa signer(randomDataGenerator, ecDsa224PublicKey, ecDsa224PrivateKey);

        PreBuilder(targetAndFiles, buildOptions, configuration);

        std::vector<std::unique_ptr<application::Input>> inputs;
        for (const auto& targetAndFile : targetAndFiles)
            inputs.push_back(inputFactory.CreateInput(targetAndFile.first, targetAndFile.second));

        for (const auto& mandatoryTarget : targets.MandatoryTargets())
        {
            bool found = false;
            for (auto& input : inputs)
                if (input->TargetName() == mandatoryTarget)
                    found = true;

            if (!found)
                throw MissingTargetException(mandatoryTarget);
        }

        application::UpgradePackBuilder builder(this->headerInfo, std::move(inputs), signer);
        PostBuilder(builder, signer, buildOptions);

        builder.WriteUpgradePack(outputFilename, fileSystem);
    }

    void UpgradePackBuilderFacade::ShowUsage(int argc, const char* argv[], const SupportedTargets& targets) const
    {
        std::cout << "Arguments: ";
        for (int i = 0; i != argc; ++i)
            std::cout << " " << argv[i];
        std::cout << std::endl;
        std::cout << "Invalid number of arguments" << std::endl;
        std::cout << argv[0] << " OutputFile [-Target1 InputFile1] [-Target2 InputFile2] [-Target3 InputFile3] [-Target4 InputFile4] ..." << std::endl;
        std::cout << "Supported Targets: " << targets << std::endl;
    }

    void UpgradePackBuilderFacade::ShowUsage(const TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, const SupportedTargets& targets) const
    {
        std::cout << "Wrong usage" << std::endl;

        std::cout << "Given targets: ";
        for (auto target : targetAndFiles)
            std::cout << " " << target.first;
        std::cout << std::endl;

        std::cout << "Given options: ";
        for (auto option : buildOptions)
            std::cout << " " << option.first;
        std::cout << std::endl;

        std::cout << "Correct Usage" << std::endl;
        std::cout << "Supported Targets: " << targets << std::endl;
    }

    int UpgradePackBuilderFacade::Result() const
    {
        return result;
    }

    void UpgradePackBuilderFacade::PreBuilder(const TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration)
    {}

    void UpgradePackBuilderFacade::PostBuilder(UpgradePackBuilder& builder, ImageSigner& signer, const std::vector<std::pair<std::string, std::string>>& buildOptions)
    {}
}
