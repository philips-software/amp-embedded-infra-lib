#include "upgrade/pack_builder/InputBinary.hpp"
#include "upgrade/pack_builder/InputHex.hpp"
#include "upgrade/pack_builder/UpgradePackInputFactory.hpp"

namespace application
{
    NoFileInput::NoFileInput(const std::string& targetName)
        : application::Input(targetName)
    {}

    std::vector<uint8_t> NoFileInput::Image() const
    {
        std::string targetName(TargetName());

        std::vector<uint8_t> result;
        result.insert(result.end(), reinterpret_cast<const uint8_t*>(targetName.data()), reinterpret_cast<const uint8_t*>(targetName.data() + targetName.size()));
        result.insert(result.end(), 8 - targetName.size(), 0);

        uint32_t encryptionAndMacMethod = 0;
        result.insert(result.end(), reinterpret_cast<const uint8_t*>(&encryptionAndMacMethod), reinterpret_cast<const uint8_t*>(&encryptionAndMacMethod + 1));

        uint32_t size = result.size() + sizeof(size);
        result.insert(result.begin(), reinterpret_cast<const uint8_t*>(&size), reinterpret_cast<const uint8_t*>(&size + 1));

        return result;
    }

    NoFileInputFactory::NoFileInputFactory(const std::string& targetName)
        : targetName(targetName)
    {}

    std::string NoFileInputFactory::TargetName() const
    {
        return targetName;
    }

    UpgradePackInputFactory::UpgradePackInputFactory(const std::vector<std::string>& supportedHexTargets,
        const std::vector<std::pair<std::string, uint32_t>>& supportedBinaryTargets,
        hal::FileSystem& fileSystem, const ImageSecurity& imageSecurity, const std::vector<NoFileInputFactory*>& otherTargets)
        : supportedHexTargets(supportedHexTargets)
        , supportedBinaryTargets(supportedBinaryTargets)
        , fileSystem(fileSystem)
        , imageSecurity(imageSecurity)
        , otherTargets(otherTargets)
    {}

    std::unique_ptr<Input> UpgradePackInputFactory::CreateInput(const std::string& targetName, const std::string& fileName)
    {
        if (std::find(supportedHexTargets.begin(), supportedHexTargets.end(), targetName) != supportedHexTargets.end())
            return std::make_unique<InputHex>(targetName, fileName, fileSystem, imageSecurity);

        for (auto targetAndAddress : supportedBinaryTargets)
            if (targetAndAddress.first == targetName)
                return std::make_unique<InputBinary>(targetName, fileName, targetAndAddress.second, fileSystem, imageSecurity);

        for (auto target : otherTargets)
            if (target->TargetName() == targetName)
                return target->CreateInput();

        throw std::runtime_error((std::string("Unknown target: ") + targetName).c_str());
    }
}
