#include "hal/generic/FileSystemGeneric.hpp"
#include <fstream>
#include <iterator>

namespace hal
{
    std::vector<std::string> FileSystemGeneric::ReadFile(const hal::filesystem::path& path)
    {
        std::ifstream input(path);
        if (!input)
            throw CannotOpenFileException(path);

        std::vector<std::string> result;

        while (input)
        {
            std::string line;
            std::getline(input, line);

            if (input)
                result.push_back(line);
        }

        return result;
    }

    void FileSystemGeneric::WriteFile(const hal::filesystem::path& path, const std::vector<std::string>& contents)
    {
        std::ofstream output(path);
        if (!output)
            throw CannotOpenFileException(path);

        for (std::string line : contents)
            output << line << std::endl;
    }

    std::vector<uint8_t> FileSystemGeneric::ReadBinaryFile(const hal::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        if (!input)
            throw CannotOpenFileException(path);

        std::vector<char> data{ std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
        return std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(data.data()), reinterpret_cast<const uint8_t*>(data.data() + data.size()));
    }

    void FileSystemGeneric::WriteBinaryFile(const hal::filesystem::path& path, const std::vector<uint8_t>& contents)
    {
        std::ofstream output(path, std::ios::binary);
        if (!output)
            throw CannotOpenFileException(path);

        std::copy(contents.data(), (contents.data() + contents.size()), std::ostreambuf_iterator<char>(output));
    }
}
