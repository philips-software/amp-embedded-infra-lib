#include  "hal/interfaces/FileSystem.hpp"

namespace hal
{
    CannotOpenFileException::CannotOpenFileException(const hal::filesystem::path& path)
        : std::runtime_error(std::string("Could not open file ") + path.string())
    {}

    EmptyFileException::EmptyFileException(const hal::filesystem::path& path)
        : std::runtime_error(std::string("File is empty ") + path.string())
    {}
}
