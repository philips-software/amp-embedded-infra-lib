#include "infra/syntax/XmlNavigator.hpp"
#include "gtest/gtest.h"

namespace
{
    infra::XmlNodeNavigatorToken doc{ "doc" };
    infra::XmlNodeNavigatorToken node{ "node" };
    infra::XmlStringAttributeNavigatorToken str{ "str" };
    infra::XmlOptionalStringAttributeNavigatorToken optionalStr{ "str" };
    infra::XmlOptionalStringAttributeNavigatorToken optionalStr2{ "str2" };
    infra::XmlIntegerAttributeNavigatorToken integer{ "int" };
    infra::XmlOptionalIntegerAttributeNavigatorToken optionalInt{ "int" };
    infra::XmlOptionalIntegerAttributeNavigatorToken optionalInt2{ "int2" };
}

TEST(XmlNavigator, access_node)
{
    auto document = R"(<node></node>)";
    infra::XmlNodeNavigator navigator{ document };
    navigator / node;
}

TEST(XmlNavigator, access_string_attribute)
{
    auto document = R"(<node str="text"></node>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_EQ("text", navigator / node / str);
    EXPECT_EQ("text", *(navigator / node / optionalStr));
    EXPECT_EQ(infra::none, navigator / node / optionalStr2);
}

TEST(XmlNavigator, access_integer_attribute)
{
    auto document = R"(<node int="5"></node>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_EQ(5, navigator / node / integer);
    EXPECT_EQ(5, *(navigator / node / optionalInt));
    EXPECT_EQ(infra::none, navigator / node / optionalInt2);
}

TEST(XmlNavigator, transform_object)
{
    struct Data
    {
        std::string str;
    };

    infra::XmlTransformObjectNavigatorToken<Data> node{ "node", [](const infra::XmlNodeNavigator& navigator)
        {
            Data result = { navigator / str };
            return result;
        } };

    auto document = R"(<node str="text"></node>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_EQ("text", (navigator / node).str);
}

TEST(XmlNavigator, transform_array)
{
    struct Data
    {
        std::string str;

        bool operator==(const Data& other) const
        {
            return str == other.str;
        }
    };

    infra::XmlTransformArrayNavigatorToken<Data> node{ "node", [](const infra::XmlNodeNavigator& navigator)
        {
            Data result = { navigator / str };
            return result;
        } };

    auto document = R"(<doc><node str="text1"></node><node str="text2"></node><node str="text3"></node></doc>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_EQ((std::vector<Data>{ { "text1" }, { "text2" }, { "text3" } }), navigator / doc / node);
}
