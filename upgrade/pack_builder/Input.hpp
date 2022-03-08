#ifndef UPGRADE_INPUT_HPP
#define UPGRADE_INPUT_HPP

#include <stdexcept>
#include <string>
#include <vector>

namespace application
{
    class TargetNameTooLongException
        : public std::runtime_error
    {
    public:
        TargetNameTooLongException(const std::string& name, int maxSize);
    };

    class Input
    {
    public:
        explicit Input(const std::string& targetName);
        virtual ~Input() = default;

        const std::string& TargetName() const;
        virtual std::vector<uint8_t> Image() const = 0;

    public:
        const std::size_t maxNameSize = 8;

    private:
        std::string targetName;
    };
}

#endif
