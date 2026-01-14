#ifndef UPGRADE_INPUT_FACTORY_HPP
#define UPGRADE_INPUT_FACTORY_HPP

#include "upgrade/pack_builder/Input.hpp"
#include <memory>
#include <optional>
#include <string>

namespace application
{
    class InputFactory
    {
    public:
        virtual std::unique_ptr<Input> CreateInput(const std::string& targetName, const std::string& fileName, std::optional<uint32_t> address) = 0;

    protected:
        ~InputFactory() = default;
    };
}

#endif
