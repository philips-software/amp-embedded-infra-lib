#ifndef HAL_WINDOWS_FILE_SYSTEM_GENERIC_HPP
#define HAL_WINDOWS_FILE_SYSTEM_GENERIC_HPP

#include "hal/interfaces/FileSystem.hpp"

namespace hal
{
    class FileSystemGeneric
        : public hal::FileSystem
    {
    public:
        virtual std::vector<std::string> ReadFile(const hal::filesystem::path& path) override;
        virtual void WriteFile(const hal::filesystem::path& path, const std::vector<std::string>& contents) override;

        virtual std::vector<uint8_t> ReadBinaryFile(const hal::filesystem::path& path) override;
        virtual void WriteBinaryFile(const hal::filesystem::path& path, const std::vector<uint8_t>& contents) override;
    };
}

#endif
