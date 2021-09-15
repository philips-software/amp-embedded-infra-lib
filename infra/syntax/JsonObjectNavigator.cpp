#include "infra/syntax/JsonObjectNavigator.hpp"

namespace infra
{
    JsonObjectNavigator::JsonObjectNavigator(const std::string& contents)
        : object(contents)
    {}

    JsonObjectNavigator::JsonObjectNavigator(infra::JsonObject& object)
        : object(object)
    {}

    JsonObjectNavigator JsonObjectNavigator::operator/(JsonObjectNavigatorToken token) const
    {
        auto subObject = object.GetOptionalObject(token.name);
        if (subObject == infra::none)
            throw std::runtime_error(("Object " + token.name + " not found").c_str());

        return { *subObject };
    }

    JsonOptionalObjectNavigator JsonObjectNavigator::operator/(JsonOptionalObjectNavigatorToken token) const
    {
        auto subObject = object.GetOptionalObject(token.name);
        if (subObject == infra::none)
            return {};

        return { *subObject };
    }

    JsonArrayNavigator JsonObjectNavigator::operator/(JsonArrayNavigatorToken token) const
    {
        auto subArray = object.GetOptionalArray(token.name);
        if (subArray == infra::none)
            throw std::runtime_error(("Array " + token.name + " not found").c_str());

        return { *subArray };
    }

    JsonOptionalArrayNavigator JsonObjectNavigator::operator/(JsonOptionalArrayNavigatorToken token) const
    {
        auto subArray = object.GetOptionalArray(token.name);
        if (subArray == infra::none)
            return {};

        return { *subArray };
    }

    std::string JsonObjectNavigator::operator/(JsonStringNavigatorToken token) const
    {
        auto member = object.GetOptionalString(token.name);
        if (member == infra::none)
            throw std::runtime_error(("String " + token.name + " not found").c_str());

        return member->ToStdString();
    }

    int32_t JsonObjectNavigator::operator/(JsonIntegerNavigatorToken token) const
    {
        auto member = object.GetOptionalInteger(token.name);
        if (member == infra::none)
            throw std::runtime_error(("Integer " + token.name + " not found").c_str());

        return *member;
    }

    bool JsonObjectNavigator::operator/(JsonBoolNavigatorToken token) const
    {
        auto member = object.GetOptionalBoolean(token.name);
        if (member == infra::none)
            throw std::runtime_error(("Boolean " + token.name + " not found").c_str());

        return *member;
    }

    JsonOptionalObjectNavigator::JsonOptionalObjectNavigator(infra::JsonObject& object)
        : navigator(infra::inPlace, object)
    {}

    JsonOptionalObjectNavigator::JsonOptionalObjectNavigator(const JsonObjectNavigator& navigator)
        : navigator(infra::inPlace, navigator)
    {}

    JsonOptionalObjectNavigator JsonOptionalObjectNavigator::operator/(JsonObjectNavigatorToken token) const
    {
        if (navigator != infra::none)
            return *navigator / token;
        else
            return {};
    }

    JsonOptionalObjectNavigator JsonOptionalObjectNavigator::operator/(JsonOptionalObjectNavigatorToken token) const
    {
        if (navigator != infra::none)
            return *navigator / token;
        else
            return {};
    }

    JsonOptionalArrayNavigator JsonOptionalObjectNavigator::operator/(JsonArrayNavigatorToken token) const
    {
        if (navigator != infra::none)
            return *navigator / token;
        else
            return {};
    }

    JsonOptionalArrayNavigator JsonOptionalObjectNavigator::operator/(JsonOptionalArrayNavigatorToken token) const
    {
        if (navigator != infra::none)
            return *navigator / token;
        else
            return {};
    }

    infra::Optional<std::string> JsonOptionalObjectNavigator::operator/(JsonStringNavigatorToken token) const
    {
        if (navigator != infra::none)
            return infra::MakeOptional(*navigator / token);
        else
            return {};
    }

    infra::Optional<int32_t> JsonOptionalObjectNavigator::operator/(JsonIntegerNavigatorToken token) const
    {
        if (navigator != infra::none)
            return infra::MakeOptional(*navigator / token);
        else
            return {};
    }

    infra::Optional<bool> JsonOptionalObjectNavigator::operator/(JsonBoolNavigatorToken token) const
    {
        if (navigator != infra::none)
            return infra::MakeOptional(*navigator / token);
        else
            return {};
    }

    JsonArrayNavigator::JsonArrayNavigator(infra::JsonArray& array)
        : array(array)
    {}

    JsonOptionalArrayNavigator::JsonOptionalArrayNavigator(infra::JsonArray& array)
        : navigator(infra::inPlace, array)
    {}

    JsonOptionalArrayNavigator::JsonOptionalArrayNavigator(const JsonArrayNavigator& navigator)
        : navigator(infra::inPlace, navigator)
    {}
}
