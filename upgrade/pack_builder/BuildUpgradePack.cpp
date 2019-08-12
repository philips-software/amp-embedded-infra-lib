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
#include <cctype>
#include <iomanip>
#include <iostream>

namespace application
{
    namespace
    {
        std::string ToLower(const std::string& str)
        {
            std::string result;
            std::transform(str.begin(), str.end(), std::back_inserter(result), [](unsigned char c) { return std::tolower(c); });
            return result;
        }

        struct UsageException
        {};
    }

    int BuildUpgradePack(const application::UpgradePackBuilder::HeaderInfo& headerInfo, const std::vector<std::string>& supportedHexTargets,
        const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets, std::string outputFilename,
        TargetAndFiles targetAndFiles, BuildOptions buildOptions, infra::JsonObject& configuration, infra::ConstByteRange aesKey,
        infra::ConstByteRange ecDsa224PublicKey, infra::ConstByteRange ecDsa224PrivateKey, const std::vector<NoFileInputFactory*>& otherTargets)
    {
        UpgradePackBuilderFacade builderFacade(headerInfo);
        builderFacade.Build(supportedHexTargets, supportedBinaryTargets, outputFilename, targetAndFiles, buildOptions, configuration, aesKey, ecDsa224PublicKey, ecDsa224PrivateKey, otherTargets);
        return builderFacade.Result();
    }

    UpgradePackBuilderFacade::UpgradePackBuilderFacade(const application::UpgradePackBuilder::HeaderInfo& headerInfo)
        : headerInfo(headerInfo)
    {
        // Initialize the MbedTLS memory pool
        unsigned char memory_buf[100000];
        mbedtls_memory_buffer_alloc_init(memory_buf, sizeof(memory_buf));
    }

    void UpgradePackBuilderFacade::Build(const std::vector<std::string>& supportedHexTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets,
        std::string outputFilename, TargetAndFiles& targetAndFiles, BuildOptions& buildOptions, infra::JsonObject& configuration, infra::ConstByteRange aesKey, infra::ConstByteRange ecDsa224PublicKey,
        infra::ConstByteRange ecDsa224PrivateKey, const std::vector<NoFileInputFactory*>& otherTargets)
    {
        try
        {
            TryBuild(supportedHexTargets, supportedBinaryTargets, outputFilename, targetAndFiles, buildOptions, configuration, aesKey, ecDsa224PublicKey, ecDsa224PrivateKey, otherTargets);
        }
        catch (UsageException&)
        {
            ShowUsage(targetAndFiles, buildOptions, supportedHexTargets, supportedBinaryTargets, otherTargets);
            result = 1;
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
            result = 1;
        }
    }

    void UpgradePackBuilderFacade::TryBuild(const std::vector<std::string>& supportedHexTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets,
        std::string outputFilename, TargetAndFiles& targetAndFiles, BuildOptions& buildOptions, infra::JsonObject& configuration, infra::ConstByteRange aesKey, infra::ConstByteRange ecDsa224PublicKey,
        infra::ConstByteRange ecDsa224PrivateKey, const std::vector<NoFileInputFactory*>& otherTargets)
    {
        hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
        hal::FileSystemGeneric fileSystem;
        application::ImageEncryptorAes imageEncryptorAes(randomDataGenerator, aesKey);
        application::UpgradePackInputFactory inputFactory(supportedHexTargets, supportedBinaryTargets, fileSystem, imageEncryptorAes, otherTargets);
        application::ImageSignerEcDsa signer(randomDataGenerator, ecDsa224PublicKey, ecDsa224PrivateKey);

        PreBuilder(targetAndFiles, buildOptions, configuration);

        std::vector<std::unique_ptr<application::Input>> inputs;
        for (auto targetAndFile : targetAndFiles)
            inputs.push_back(inputFactory.CreateInput(targetAndFile.first, targetAndFile.second));

        application::UpgradePackBuilder builder(this->headerInfo, std::move(inputs), signer);
        PostBuilder(builder, signer, buildOptions);

        builder.WriteUpgradePack(outputFilename, fileSystem);
    }

    void UpgradePackBuilderFacade::ShowUsage(int argc, const char* argv[], const std::vector<std::string>& supportedHexTargets,
        const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets, const std::vector<NoFileInputFactory*>& otherTargets) const
    {
        std::cout << "Arguments: ";
        for (int i = 0; i != argc; ++i)
            std::cout << " " << argv[i];
        std::cout << std::endl;
        std::cout << "Invalid number of arguments" << std::endl;
        std::cout << argv[0] << " OutputFile [-Target1 InputFile1] [-Target2 InputFile2] [-Target3 InputFile3] [-Target4 InputFile4] ..." << std::endl;
        std::cout << "Targets: ";

        for (auto target : supportedHexTargets)
            std::cout << target << " ";
        for (auto targetAndAddress : supportedBinaryTargets)
            std::cout << targetAndAddress.first << " ";
        for (auto target : otherTargets)
            std::cout << target->TargetName() << " ";
        std::cout << std::endl;
    }

    void UpgradePackBuilderFacade::ShowUsage(TargetAndFiles& targetAndFiles, BuildOptions& buildOptions,
        const std::vector<std::string>& supportedHexTargets, const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets,
        const std::vector<NoFileInputFactory*>& otherTargets) const
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
        std::cout << "Available Targets: ";
        for (auto target : supportedHexTargets)
            std::cout << target << " ";
        for (auto targetAndAddress : supportedBinaryTargets)
            std::cout << targetAndAddress.first << " ";
        for (auto target : otherTargets)
            std::cout << target->TargetName() << " ";
        std::cout << std::endl;
    }

    int UpgradePackBuilderFacade::Result() const
    {
        return result;
    }

    void UpgradePackBuilderFacade::PreBuilder(TargetAndFiles& targetAndFiles, const BuildOptions& buildOptions, infra::JsonObject& configuration)
    {}

    void UpgradePackBuilderFacade::PostBuilder(UpgradePackBuilder& builder, ImageSigner& signer, const std::vector<std::pair<std::string, std::string>>& buildOptions)
    {}
}
