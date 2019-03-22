#include "hal/interfaces/test_doubles/FileSystemStub.hpp"
#include "gtest/gtest.h"

namespace hal
{
    FileSystemStub::FileSystemStub(const hal::filesystem::path& path, const std::vector<std::string>& contents)
    {
        files[path] = contents;
    }

    FileSystemStub::FileSystemStub(const hal::filesystem::path& path, const std::vector<uint8_t>& contents)
    {
        binaryFiles[path] = contents;
    }

    std::vector<std::string> FileSystemStub::ReadFile(const hal::filesystem::path& path)
    {
        return files[path];
    }

    void FileSystemStub::WriteFile(const hal::filesystem::path& path, const std::vector<std::string>& contents)
    {
        files[path] = contents;
    }

    std::vector<uint8_t> FileSystemStub::ReadBinaryFile(const hal::filesystem::path& path)
    {
        return binaryFiles[path];
    }

    void FileSystemStub::WriteBinaryFile(const hal::filesystem::path& path, const std::vector<uint8_t>& contents)
    {
        binaryFiles[path] = contents;
    }
}
