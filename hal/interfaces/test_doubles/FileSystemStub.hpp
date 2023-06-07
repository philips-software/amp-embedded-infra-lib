#ifndef HAL_STUB_FILE_SYSTEM_STUB_HPP
#define HAL_STUB_FILE_SYSTEM_STUB_HPP

#include "hal/interfaces/FileSystem.hpp"
#include <map>

namespace hal
{
    class FileSystemStub
        : public hal::FileSystem
    {
    public:
        FileSystemStub() = default;
        FileSystemStub(const hal::filesystem::path& path, const std::vector<std::string>& contents);
        FileSystemStub(const hal::filesystem::path& path, const std::vector<uint8_t>& contents);

        std::vector<std::string> ReadFile(const hal::filesystem::path& path) override;
        void WriteFile(const hal::filesystem::path& path, const std::vector<std::string>& contents) override;

        std::vector<uint8_t> ReadBinaryFile(const hal::filesystem::path& path) override;
        void WriteBinaryFile(const hal::filesystem::path& path, const std::vector<uint8_t>& contents) override;

        std::map<hal::filesystem::path, std::vector<std::string>> files;
        std::map<hal::filesystem::path, std::vector<uint8_t>> binaryFiles;
    };
}

#endif
