#include "infra/syntax/XmlNavigator.hpp"
#include <stdexcept>

namespace infra
{
    XmlNodeNavigator::XmlNodeNavigator(const std::string& contents)
    {
        document = pugi::xml_document();
        pugi::xml_parse_result result = document.load_string(contents.c_str());
        if (!result)
            throw std::runtime_error("Document failed to load");

        node = document;
    }

    XmlNodeNavigator::XmlNodeNavigator(const pugi::xml_node& node)
        : node(node)
    {}

    XmlNodeNavigator XmlNodeNavigator::operator/(XmlNodeNavigatorToken token) const
    {
        auto child = node.child(token.name.c_str());
        if (child == pugi::xml_node())
            throw std::runtime_error(("Child " + token.name + " not found").c_str());

        return { child };
    }

    std::string XmlNodeNavigator::operator/(XmlStringAttributeNavigatorToken token) const
    {
        for (auto attribute: node.attributes())
            if (attribute.name() == token.name)
                return attribute.value();

        throw std::runtime_error(("Attribute " + token.name + " not found").c_str());
    }

    infra::Optional<std::string> XmlNodeNavigator::operator/(XmlOptionalStringAttributeNavigatorToken token) const
    {
        for (auto attribute = node.attributes_begin(); attribute != node.attributes_end(); attribute = ++attribute)
            if (attribute->name() == token.name)
                return infra::MakeOptional(std::string(attribute->value()));

        return infra::none;
    }

    int32_t XmlNodeNavigator::operator/(XmlIntegerAttributeNavigatorToken token) const
    {
        for (auto attribute: node.attributes())
            if (attribute.name() == token.name)
                return attribute.as_int();

        throw std::runtime_error(("Attribute " + token.name + " not found").c_str());
    }

    infra::Optional<int32_t> XmlNodeNavigator::operator/(XmlOptionalIntegerAttributeNavigatorToken token) const
    {
        for (auto attribute: node.attributes())
            if (attribute.name() == token.name)
                return infra::MakeOptional(attribute.as_int());

        return infra::none;
    }
}
