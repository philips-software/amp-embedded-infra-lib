#ifndef UPGRADE_INPUT_COMMAND_HPP
#define UPGRADE_INPUT_COMMAND_HPP

#include "infra/util/ByteRange.hpp"
#include "upgrade/pack_builder/Input.hpp"

namespace application
{
    class InputCommand
        : public Input
    {
    public:
        explicit InputCommand(const std::string& targetName, const infra::ConstByteRange& parameters = {});

        std::vector<uint8_t> Image() const override;

    private:
        std::vector<uint8_t> image;
    };
}

#endif
