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

    struct Data
    {
        std::string str;

        bool operator==(const Data& other) const
        {
            return str == other.str;
        }
    };
}

TEST(XmlNavigator, loading_failed)
{
    auto document = R"(this is not xml)";
    EXPECT_THROW(infra::XmlNodeNavigator{ document }, std::runtime_error);
}

TEST(XmlNavigator, access_node)
{
    auto document = R"(<node></node>)";
    infra::XmlNodeNavigator navigator{ document };
    navigator / node;
}

TEST(XmlNavigator, accessing_missing_node_throws)
{
    auto document = R"(<othernode></othernode>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_THROW(navigator / node, std::runtime_error);
}

TEST(XmlNavigator, access_string_attribute)
{
    auto document = R"(<node other="x" str="text"></node>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_EQ("text", navigator / node / str);
    EXPECT_EQ("text", *(navigator / node / optionalStr));
    EXPECT_EQ(infra::none, navigator / node / optionalStr2);
}

TEST(XmlNavigator, accessing_missing_string_throws)
{
    auto document = R"(<node></node>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_THROW(navigator / node / str, std::exception);
}

TEST(XmlNavigator, access_integer_attribute)
{
    auto document = R"(<node int="5"></node>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_EQ(5, navigator / node / integer);
    EXPECT_EQ(5, *(navigator / node / optionalInt));
    EXPECT_EQ(infra::none, navigator / node / optionalInt2);
}

TEST(XmlNavigator, accessing_missing_integer_throws)
{
    auto document = R"(<node></node>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_THROW(navigator / node / integer, std::runtime_error);
}

TEST(XmlNavigator, transform_object)
{
    infra::XmlTransformObjectNavigatorToken<Data> node{ "node", [](const infra::XmlNodeNavigator& navigator)
        {
            Data result = { navigator / str };
            return result;
        } };

    auto document = R"(<node str="text"></node>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_EQ("text", (navigator / node).str);
}

TEST(XmlNavigator, missing_object_throws)
{
    infra::XmlTransformObjectNavigatorToken<Data> node{ "node", [](const infra::XmlNodeNavigator& navigator)
        {
            Data result = { navigator / str };
            return result;
        } };

    auto document = R"(<node></node>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_THROW(navigator / node, std::runtime_error);
}

TEST(XmlNavigator, missing_node_in_navigation_throws)
{
    infra::XmlTransformObjectNavigatorToken<Data> node{ "node", [](const infra::XmlNodeNavigator& navigator)
        {
            Data result = { navigator / str };
            return result;
        } };

    auto document = R"(<othernode></othernode>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_THROW(navigator / node, std::runtime_error);
}

TEST(XmlNavigator, transform_array)
{
    infra::XmlTransformArrayNavigatorToken<Data> node{ "node", [](const infra::XmlNodeNavigator& navigator)
        {
            Data result = { navigator / str };
            return result;
        } };

    auto document = R"(<doc><node str="text1"></node><node str="text2"></node><node str="text3"></node></doc>)";
    infra::XmlNodeNavigator navigator{ document };
    EXPECT_EQ((std::vector<Data>{ { "text1" }, { "text2" }, { "text3" } }), navigator / doc / node);
}
