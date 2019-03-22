#include "upgrade/pack_builder/Input.hpp"

namespace application
{
    Input::Input(const std::string& targetName)
        : targetName(targetName)
    {}

    std::string Input::TargetName() const
    {
        return targetName;
    }
}
