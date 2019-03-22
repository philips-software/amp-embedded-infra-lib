#ifndef UPGRADE_PACK_BUILDER_LIBRARY_INPUT_HPP
#define UPGRADE_PACK_BUILDER_LIBRARY_INPUT_HPP

#include "upgrade/pack/UpgradePackHeader.hpp"
#include <vector>

namespace application
{
    class Input
    {
    public:
        explicit Input(const std::string& targetName);
        virtual ~Input() = default;

        std::string TargetName() const;
        virtual std::vector<uint8_t> Image() const = 0;

    private:
        std::string targetName;
    };
}

#endif
