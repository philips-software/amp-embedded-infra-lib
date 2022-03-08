#ifndef UPGRADE_INPUT_COMMAND_HPP
#define UPGRADE_INPUT_COMMAND_HPP

#include "upgrade/pack_builder/Input.hpp"

namespace application
{
    class InputCommand
        : public Input
    {
    public:
        explicit InputCommand(const std::string& targetName);

        virtual std::vector<uint8_t> Image() const;
    };
}

#endif
