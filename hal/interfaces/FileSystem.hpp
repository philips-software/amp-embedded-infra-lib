#ifndef HAL_INTERFACE_FILE_SYSTEM_HPP
#define HAL_INTERFACE_FILE_SYSTEM_HPP

#include <cstdint>
#include <exception>
#include <filesystem>
#include <string>
#include <vector>

namespace hal
{
    namespace filesystem = std::experimental::filesystem;

    struct CannotOpenFileException
        : std::runtime_error
    {
        explicit CannotOpenFileException(const hal::filesystem::path& path);
    };

    struct EmptyFileException
        : std::runtime_error
    {
        explicit EmptyFileException(const hal::filesystem::path& path);
    };

    class FileSystem
    {
    public:
        virtual std::vector<std::string> ReadFile(const hal::filesystem::path& path) = 0;
        virtual void WriteFile(const hal::filesystem::path& path, const std::vector<std::string>& contents) = 0;

        virtual std::vector<uint8_t> ReadBinaryFile(const hal::filesystem::path& path) = 0;
        virtual void WriteBinaryFile(const hal::filesystem::path& path, const std::vector<uint8_t>& contents) = 0;

    protected:
        ~FileSystem() = default;
    };
}

#endif
