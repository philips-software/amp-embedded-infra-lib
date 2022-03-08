#include "upgrade/pack_builder/InputCommand.hpp"

namespace application
{
    InputCommand::InputCommand(const std::string& targetName)
        : application::Input(targetName)
    {}

    std::vector<uint8_t> InputCommand::Image() const
    {
        std::string targetName(TargetName());

        std::vector<uint8_t> result;
        result.insert(result.end(), reinterpret_cast<const uint8_t*>(targetName.data()), reinterpret_cast<const uint8_t*>(targetName.data() + targetName.size()));
        result.insert(result.end(), Input::maxNameSize - targetName.size(), 0);

        uint32_t encryptionAndMacMethod = 0;
        result.insert(result.end(), reinterpret_cast<const uint8_t*>(&encryptionAndMacMethod), reinterpret_cast<const uint8_t*>(&encryptionAndMacMethod + 1));

        uint32_t size = result.size() + sizeof(size);
        result.insert(result.begin(), reinterpret_cast<const uint8_t*>(&size), reinterpret_cast<const uint8_t*>(&size + 1));

        return result;
    }
}
