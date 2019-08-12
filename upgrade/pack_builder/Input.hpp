#ifndef UPGRADE_PACK_BUILDER_LIBRARY_INPUT_HPP
#define UPGRADE_PACK_BUILDER_LIBRARY_INPUT_HPP

#include "upgrade/pack/UpgradePackHeader.hpp"
#include <vector>

namespace application
{
    class TargetNameTooLongException
        : public std::runtime_error
    {
    public:
        TargetNameTooLongException(const std::string& name, int maxSize);
    };

    class Input
    {
    public:
        explicit Input(const std::string& targetName);
        virtual ~Input() = default;

        std::string TargetName() const;
        virtual std::vector<uint8_t> Image() const = 0;

    public:
        const int maxNameSize = 8;

    private:
        std::string targetName;
    };
}

#endif
