#ifndef UPGRADE_PACK_BUILDER_LIBRARY_INPUT_FACTORY_HPP
#define UPGRADE_PACK_BUILDER_LIBRARY_INPUT_FACTORY_HPP

#include "upgrade/pack_builder/Input.hpp"
#include <memory>
#include <string>

namespace application
{
    class InputFactory
    {
    public:
        virtual std::unique_ptr<Input> CreateInput(const std::string& targetName, const std::string& fileName) = 0;

    protected:
        ~InputFactory() = default;
    };
}

#endif
