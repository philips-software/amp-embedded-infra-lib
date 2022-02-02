#include <algorithm>
#include "upgrade/pack_builder/InputBinary.hpp"
#include "upgrade/pack_builder/InputCommand.hpp"
#include "upgrade/pack_builder/InputElf.hpp"
#include "upgrade/pack_builder/InputHex.hpp"
#include "upgrade/pack_builder/UpgradePackInputFactory.hpp"

namespace application
{
    UpgradePackInputFactory::UpgradePackInputFactory(hal::FileSystem& fileSystem, const SupportedTargets& targets, const ImageSecurity& imageSecurity)
        : fileSystem(fileSystem)
        , targets(targets)
        , imageSecurity(imageSecurity)
    {}

    std::unique_ptr<Input> UpgradePackInputFactory::CreateInput(const std::string& targetName, const std::string& fileName)
    {
        if (std::any_of(targets.CmdTargets().cbegin(), targets.CmdTargets().cend(), [targetName](auto& string) { return string == targetName; }))
            return std::make_unique<InputCommand>(targetName);

        if (std::any_of(targets.HexTargets().cbegin(), targets.HexTargets().cend(), [targetName](auto& string) { return string == targetName; }))
            return std::make_unique<InputHex>(targetName, fileName, fileSystem, imageSecurity);

        for (const auto& target : targets.ElfTargets())
            if (target.first == targetName)
                return std::make_unique<InputElf>(targetName, fileName, target.second, fileSystem, imageSecurity);

        for (const auto& target : targets.BinTargets())
            if (target.first == targetName)
                return std::make_unique<InputBinary>(targetName, fileName, target.second, fileSystem, imageSecurity);

        throw std::runtime_error((std::string("Unknown target: ") + targetName).c_str());
    }
}
