#include "infra/syntax/JsonObjectNavigator.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <optional>
#include <type_traits>
#include <variant>

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

struct JsonObjectNavigatorTestParam
{
    std::variant<
        infra::JsonStringNavigatorToken,
        infra::JsonOptionalStringNavigatorToken,
        infra::JsonIntegerNavigatorToken,
        infra::JsonBoolNavigatorToken>
        navigator;

    std::variant<
        std::monostate,
        std::optional<std::string>,
        std::string,
        int32_t,
        bool>
        expected;
};

class JsonObjectNavigatorTest
    : public testing::TestWithParam<JsonObjectNavigatorTestParam>
{
};

TEST_P(JsonObjectNavigatorTest, object_navigator)
{
    infra::JsonObject object(R"({ "subobject" : { "nestedBool": true, "nestedString": "test", "nestedInt": 42 } })");
    infra::JsonObjectNavigator objectNavigator(object);
    infra::JsonObjectNavigatorToken sub{ "subobject" };
    infra::JsonObjectNavigatorToken nonSub{ "nonSub" };

    const JsonObjectNavigatorTestParam parameter = GetParam();

    std::visit([&](const auto& navigator)
        {
            const auto actual = objectNavigator / sub / navigator;
            const auto expected = std::get<std::remove_cv_t<decltype(actual)>>(parameter.expected);
            EXPECT_THAT(actual, expected);
        },
        parameter.navigator);
}

INSTANTIATE_TEST_SUITE_P(JsonOptionalObjectNavigatorTestValidTokenPath, JsonObjectNavigatorTest,
    testing::Values(
        JsonObjectNavigatorTestParam{ infra::JsonStringNavigatorToken{ "nestedString" }, std::string{ "test" } },
        JsonObjectNavigatorTestParam{ infra::JsonOptionalStringNavigatorToken{ "nestedString" }, std::optional<std::string>{ "test" } },
        JsonObjectNavigatorTestParam{ infra::JsonIntegerNavigatorToken{ "nestedInt" }, 42 },
        JsonObjectNavigatorTestParam{ infra::JsonBoolNavigatorToken{ "nestedBool" }, true }));

class JsonOptionalObjectNavigatorTest
    : public testing::TestWithParam<JsonObjectNavigatorTestParam>
{
};

TEST_P(JsonOptionalObjectNavigatorTest, optional_object_optional_navigator)
{
    infra::JsonObject object(R"({ "root" : { "nonoptional" : {"subobject" : { "nestedBool": true, "nestedString": "test", "nestedInt": 42 } } } })");
    infra::JsonObjectNavigator objectNavigator(object);
    infra::JsonOptionalObjectNavigatorToken root{ "root" };
    infra::JsonObjectNavigatorToken nonoptional{ "nonoptional" };
    infra::JsonOptionalObjectNavigatorToken sub{ "subobject" };
    infra::JsonOptionalObjectNavigatorToken nonSub{ "nonSub" };

    const JsonObjectNavigatorTestParam parameter = GetParam();

    std::visit([&](const auto& navigator)
        {
            if (std::holds_alternative<std::monostate>(parameter.expected))
            {
                const auto actualOptional = objectNavigator / root / nonoptional / nonSub / navigator;
                EXPECT_THAT(actualOptional, testing::IsFalse());
            }
            else
            {
                const auto actualOptional = objectNavigator / root / nonoptional / sub / navigator;
                const auto expected = std::get<typename decltype(actualOptional)::value_type>(parameter.expected);
                EXPECT_THAT(actualOptional, testing::Optional(expected));
            }
        },
        parameter.navigator);
}

INSTANTIATE_TEST_SUITE_P(JsonOptionalObjectNavigatorTestValidTokenPath, JsonOptionalObjectNavigatorTest,
    testing::Values(
        JsonObjectNavigatorTestParam{ infra::JsonStringNavigatorToken{ "nestedString" }, std::string{ "test" } },
        JsonObjectNavigatorTestParam{ infra::JsonOptionalStringNavigatorToken{ "nestedString" }, std::string{ "test" } },
        JsonObjectNavigatorTestParam{ infra::JsonIntegerNavigatorToken{ "nestedInt" }, 42 },
        JsonObjectNavigatorTestParam{ infra::JsonBoolNavigatorToken{ "nestedBool" }, true }));

INSTANTIATE_TEST_SUITE_P(JsonOptionalObjectNavigatorTestInvalidTokenPath, JsonOptionalObjectNavigatorTest,
    testing::Values(
        JsonObjectNavigatorTestParam{ infra::JsonStringNavigatorToken{ "nestedString" }, std::monostate{} },
        JsonObjectNavigatorTestParam{ infra::JsonOptionalStringNavigatorToken{ "nestedString" }, std::monostate{} },
        JsonObjectNavigatorTestParam{ infra::JsonIntegerNavigatorToken{ "nestedInt" }, std::monostate{} },
        JsonObjectNavigatorTestParam{ infra::JsonBoolNavigatorToken{ "nestedBool" }, std::monostate{} }));
