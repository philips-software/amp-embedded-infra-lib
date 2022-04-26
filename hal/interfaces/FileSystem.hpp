#ifndef HAL_INTERFACE_FILE_SYSTEM_HPP
#define HAL_INTERFACE_FILE_SYSTEM_HPP

#include <cstdint>
#include <string>
#include <vector>

// Detect proper filesystem header and namespace; first by using
// feature testing macros, then by using include file testing.
#if defined(__cpp_lib_filesystem)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#elif defined(__cpp_lib_experimental_filesystem)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1

#if defined(_MSC_VER) && _MSC_VER > 1900
// For now we can't compile for C++17 since not all of our platform
// compilers support that standard. As long as we can't, we need to
// silence the experimental filesystem warning under VS2017+.
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#elif !defined(__has_include)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#elif __has_include(<filesystem>)
#if defined(_MSC_VER)
// Work around for VisualStudio 2015 where include
// and namespace don't match.
#if __has_include(<yvals_core.h>)
#include <yvals_core.h>

#if defined(_HAS_CXX17) && _HAS_CXX17
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#endif
#endif

#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#endif

#else // #ifdef _MSC_VER
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#endif
#elif __has_include(<experimental/filesystem>)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#else
#error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#endif

// We previously determined that we need the exprimental version
#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>

namespace hal
{
    namespace filesystem = std::experimental::filesystem;
}
#else
#include <filesystem>

namespace hal
{
#ifdef __APPLE__
    // Work around for XCode 11 with target version 10.15 where the namespace is not std::filesystem
    namespace filesystem = std::__fs::filesystem;
#else
    namespace filesystem = std::filesystem;
#endif
}
#endif

namespace hal
{
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
