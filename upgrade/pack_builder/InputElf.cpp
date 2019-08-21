#include "upgrade/pack_builder/InputBinary.hpp"
#include "upgrade/pack_builder/InputElf.hpp"
#include "upgrade/pack_builder/Elf.hpp"

namespace application
{
    InputElf::InputElf(const std::string& targetName, const std::string& fileName, uint32_t offset, hal::FileSystem& fileSystem, const ImageSecurity& imageSecurity)
        : Input(targetName)
        , offset(offset)
        , imageSecurity(imageSecurity)
    {
        contents.AddElf(fileSystem.ReadBinaryFile(fileName), offset, fileName);
    }

    std::vector<uint8_t> InputElf::Image() const
    {
        std::vector<uint8_t> binary;
        uint32_t startAddress = 0;
        std::tie(binary, startAddress) = Linearize(contents.Memory());
        InputBinary inputBinary(TargetName(), binary, startAddress, imageSecurity);

        return inputBinary.Image();
    }

    std::pair<std::vector<uint8_t>, uint32_t> InputElf::Linearize(const application::SparseVector<uint8_t>& data) const
    {
        uint32_t startAddress = (*data.begin()).first;
        std::vector<uint8_t> result;

        for (auto i : data)
        {
            uint32_t address = i.first;
            if (result.size() < address + 1 - startAddress)
                result.resize(address + 1 - startAddress);

            result[address - startAddress] = i.second;
        }

        return std::make_pair(result, startAddress);
    }
}
