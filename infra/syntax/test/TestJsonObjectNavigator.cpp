#include "infra/syntax/JsonObjectNavigator.hpp"
#include "gtest/gtest.h"

TEST(JsonObjectNavigatorTest, construct_with_string)
{
    std::string jsonString = R"({ "key" : "value", "enabled" : true })";
    infra::JsonObjectNavigator objectNavigator(jsonString);

    EXPECT_EQ("value", objectNavigator / infra::JsonStringNavigatorToken{ "key" });
}

TEST(JsonObjectNavigatorTest, string_navigator_token)
{
    infra::JsonObject object(R"({ "key" : "value", "enabled" : true, "subobject" : { "nested": true } })");
    infra::JsonObjectNavigator objectNavigator(object);

    EXPECT_EQ("value", objectNavigator / infra::JsonStringNavigatorToken{ "key" });
}

TEST(JsonObjectNavigatorTest, integer_navigator_token)
{
    infra::JsonObject object(R"({ "int" : 32, "enabled" : true, "subobject" : { "nested": true } })");
    infra::JsonObjectNavigator objectNavigator(object);

    EXPECT_EQ(32, objectNavigator / infra::JsonIntegerNavigatorToken{ "int" });
}

TEST(JsonObjectNavigatorTest, bool_navigator_token)
{
    infra::JsonObject object(R"({ "int" : 32, "enabled" : true, "subobject" : { "nested": true } })");
    infra::JsonObjectNavigator objectNavigator(object);

    EXPECT_TRUE(objectNavigator / infra::JsonBoolNavigatorToken{ "enabled" });
}

TEST(JsonObjectNavigatorTest, object_navigator_token)
{
    infra::JsonObject object(R"({ "key" : "value", "enabled" : true, "subobject" : { "nested": true } })");
    infra::JsonObjectNavigator objectNavigator(object);

    auto subObj = objectNavigator / infra::JsonObjectNavigatorToken{ "subobject" };
    EXPECT_EQ(true, subObj / infra::JsonBoolNavigatorToken{ "nested" });
}

TEST(JsonObjectNavigatorTest, optional_object_navigator_token_not_exeist)
{
    infra::JsonObject object(R"({ "key" : "value", "enabled" : true, "subobject" : { "nested": true } })");
    infra::JsonObjectNavigator objectNavigator(object);
    infra::JsonOptionalObjectNavigatorToken nonSub{ "nonSub" };

    EXPECT_EQ(std::nullopt, objectNavigator / nonSub / infra::JsonBoolNavigatorToken{ "nested" });
}

TEST(JsonObjectNavigatorTest, optional_object_navigator_token_exeist)
{
    infra::JsonObject object(R"({ "key" : "value", "enabled" : true, "subobject" : { "nested": true } })");
    infra::JsonObjectNavigator objectNavigator(object);
    infra::JsonOptionalObjectNavigatorToken sub{ "subobject" };

    EXPECT_EQ(true, objectNavigator / sub / infra::JsonBoolNavigatorToken{ "nested" });
}
