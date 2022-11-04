#ifndef UPGRADE_INPUT_ELF_HPP
#define UPGRADE_INPUT_ELF_HPP

#include "hal/interfaces/FileSystem.hpp"
#include "upgrade/pack_builder/BinaryObject.hpp"
#include "upgrade/pack_builder/ImageSecurity.hpp"
#include "upgrade/pack_builder/Input.hpp"

namespace application
{
    class InputElf
        : public Input
    {
    public:
        InputElf(const std::string& targetName, const std::string& fileName, uint32_t offset, hal::FileSystem& fileSystem, const ImageSecurity& imageSecurity);

        virtual std::vector<uint8_t> Image() const override;

    private:
        std::pair<std::vector<uint8_t>, uint32_t> Linearize(const application::SparseVector<uint8_t>& data) const;

    private:
        const ImageSecurity& imageSecurity;
        application::BinaryObject contents;
    };
}

#endif
