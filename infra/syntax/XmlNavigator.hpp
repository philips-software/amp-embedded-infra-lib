#ifndef INFRA_XML_OBJECT_NAVIGATOR_HPP
#define INFRA_XML_OBJECT_NAVIGATOR_HPP

#include "infra/util/Optional.hpp"
#include "pugixml.hpp"
#include <functional>
#include <stdexcept>

namespace infra
{
    class XmlNodeNavigator;

    struct XmlNodeNavigatorToken
    {
        std::string name;
    };

    struct XmlStringAttributeNavigatorToken
    {
        std::string name;
    };

    struct XmlOptionalStringAttributeNavigatorToken
    {
        std::string name;
    };

    struct XmlIntegerAttributeNavigatorToken
    {
        std::string name;
    };

    struct XmlOptionalIntegerAttributeNavigatorToken
    {
        std::string name;
    };

    template<class Result>
    struct XmlTransformObjectNavigatorToken
    {
        std::string name;
        std::function<Result(const XmlNodeNavigator& navigator)> transformation;
    };

    template<class Result>
    struct XmlTransformArrayNavigatorToken
    {
        std::string name;
        std::function<Result(const XmlNodeNavigator& navigator)> transformation;
    };

    class XmlNodeNavigator
    {
    public:
        explicit XmlNodeNavigator(const std::string& contents);
        explicit XmlNodeNavigator(const pugi::xml_node& node);

        XmlNodeNavigator operator/(const XmlNodeNavigatorToken& token) const;
        std::string operator/(const XmlStringAttributeNavigatorToken& token) const;
        std::optional<std::string> operator/(const XmlOptionalStringAttributeNavigatorToken& token) const;
        int32_t operator/(const XmlIntegerAttributeNavigatorToken& token) const;
        std::optional<int32_t> operator/(const XmlOptionalIntegerAttributeNavigatorToken& token) const;

        template<class Result>
        Result operator/(const XmlTransformObjectNavigatorToken<Result>& token) const;
        template<class Result>
        std::vector<Result> operator/(const XmlTransformArrayNavigatorToken<Result>& token) const;

    private:
        pugi::xml_document document{ pugi::xml_document() };
        pugi::xml_node node;
    };

    class XmlNavigationError
        : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    //// Implementation    ////

    template<class Result>
    Result XmlNodeNavigator::operator/(const XmlTransformObjectNavigatorToken<Result>& token) const
    {
        auto child = node.child(token.name.c_str());
        if (child == pugi::xml_node())
            throw XmlNavigationError(("Child " + token.name + " not found").c_str());

        return token.transformation(XmlNodeNavigator{ child });
    }

    template<class Result>
    std::vector<Result> XmlNodeNavigator::operator/(const XmlTransformArrayNavigatorToken<Result>& token) const
    {
        std::vector<Result> result;

        for (auto child : node.children(token.name.c_str()))
            result.push_back(token.transformation(XmlNodeNavigator{ child }));

        return result;
    }
}

#endif
