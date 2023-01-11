#ifndef UPGRADE_UPGRADE_PACK_INPUT_FACTORY_HPP
#define UPGRADE_UPGRADE_PACK_INPUT_FACTORY_HPP

#include "hal/interfaces/FileSystem.hpp"
#include "upgrade/pack_builder/ImageSecurity.hpp"
#include "upgrade/pack_builder/InputFactory.hpp"
#include "upgrade/pack_builder/SupportedTargets.hpp"

namespace application
{
    class UnknownTargetException
        : public std::runtime_error
    {
    public:
        explicit UnknownTargetException(const std::string& target)
            : std::runtime_error(std::string("Unknown target '") + target + "'")
        {}
    };

    class UpgradePackInputFactory
        : public InputFactory
    {
    public:
        UpgradePackInputFactory(hal::FileSystem& fileSystem, const SupportedTargets& targets, const ImageSecurity& imageSecurity);

        virtual std::unique_ptr<Input> CreateInput(const std::string& targetName, const std::string& fileName, infra::Optional<uint32_t> address) override;

    private:
        hal::FileSystem& fileSystem;
        const SupportedTargets& targets;
        const ImageSecurity& imageSecurity;
    };
}

#endif
