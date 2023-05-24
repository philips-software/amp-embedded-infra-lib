#ifndef HAL_GENERIC_FILE_SYSTEM_GENERIC_HPP
#define HAL_GENERIC_FILE_SYSTEM_GENERIC_HPP

#include "hal/interfaces/FileSystem.hpp"

namespace hal
{
    class FileSystemGeneric
        : public hal::FileSystem
    {
    public:
        std::vector<std::string> ReadFile(const hal::filesystem::path& path) override;
        void WriteFile(const hal::filesystem::path& path, const std::vector<std::string>& contents) override;

        std::vector<uint8_t> ReadBinaryFile(const hal::filesystem::path& path) override;
        void WriteBinaryFile(const hal::filesystem::path& path, const std::vector<uint8_t>& contents) override;
    };
}

#endif
