#include "infra/syntax/XmlNavigator.hpp"

namespace infra
{
    XmlNodeNavigator::XmlNodeNavigator(const std::string& contents)
    {
        pugi::xml_parse_result result = document.load_string(contents.c_str());
        if (!result)
            throw XmlNavigationError("Document failed to load");

        node = document;
    }

    XmlNodeNavigator::XmlNodeNavigator(const pugi::xml_node& node)
        : node(node)
    {}

    XmlNodeNavigator XmlNodeNavigator::operator/(const XmlNodeNavigatorToken& token) const
    {
        auto child = node.child(token.name.c_str());
        if (child == pugi::xml_node())
            throw XmlNavigationError(("Child " + token.name + " not found").c_str());

        return XmlNodeNavigator{ child };
    }

    std::string XmlNodeNavigator::operator/(const XmlStringAttributeNavigatorToken& token) const
    {
        for (auto attribute : node.attributes())
            if (attribute.name() == token.name)
                return attribute.value();

        throw XmlNavigationError(("Attribute " + token.name + " not found").c_str());
    }

    infra::Optional<std::string> XmlNodeNavigator::operator/(const XmlOptionalStringAttributeNavigatorToken& token) const
    {
        for (auto attribute = node.attributes_begin(); attribute != node.attributes_end(); attribute = ++attribute)
            if (attribute->name() == token.name)
                return std::make_optional(std::string(attribute->value()));

        return infra::none;
    }

    int32_t XmlNodeNavigator::operator/(const XmlIntegerAttributeNavigatorToken& token) const
    {
        for (auto attribute : node.attributes())
            if (attribute.name() == token.name)
                return attribute.as_int();

        throw XmlNavigationError(("Attribute " + token.name + " not found").c_str());
    }

    infra::Optional<int32_t> XmlNodeNavigator::operator/(const XmlOptionalIntegerAttributeNavigatorToken& token) const
    {
        for (auto attribute : node.attributes())
            if (attribute.name() == token.name)
                return std::make_optional(attribute.as_int());

        return infra::none;
    }
}
