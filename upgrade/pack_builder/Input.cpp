#include "upgrade/pack_builder/Input.hpp"

namespace application
{
    TargetNameTooLongException::TargetNameTooLongException(const std::string& name, int maxSize)
        : name(name)
        , maxSize(maxSize)
    {}

    Input::Input(const std::string& targetName)
        : targetName(targetName)
    {
        if (targetName.size() > maxNameSize)
            throw TargetNameTooLongException(targetName, maxNameSize);
    }

    std::string Input::TargetName() const
    {
        return targetName;
    }
}
