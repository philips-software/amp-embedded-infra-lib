#include "gtest/gtest.h"
#include "infra/syntax/JsonFileReader.hpp"

namespace
{
    const std::vector<std::string> emptyJson = {};
    const std::vector<std::string> validJson = {
        R"({)",
        R"(    "string": "value",)",
        R"(    "integer": 42,)",
        R"(    "boolean": true,)",
        R"(    "array": [)",
        R"(        "some",)",
        R"(        "data")",
        R"(    ],)",
        R"(    "object": {)",
        R"(        "nested":true)",
        R"(    })",
        R"(})",
    };

    const std::vector<std::string> invalidJson = {
        R"({)",
        R"(    "string": "value",)",
        R"(    "integer": 42)",
        R"(    "boolean": true,)",
        R"(})",
    };

    const std::vector<std::string> invalidNestedJson = {
        R"({)",
        R"(    "key0": "value0",)",
        R"(    "object": {)",
        R"(        "key1":"value1",)",
        R"(        "key2":"value2",)",
        R"(        "key3":"value3",)",
        R"(    })",
        R"(})",
    };
}

class TestJsonFileReader
    : public testing::Test
{
public:
    infra::Optional<infra::JsonFileReader> jsonFileReader;
};

TEST_F(TestJsonFileReader, read_valid_json_from_file)
{
    jsonFileReader.Emplace(validJson);

    EXPECT_EQ(jsonFileReader->GetJsonObject().GetString("string"), "value");
    EXPECT_EQ(jsonFileReader->GetJsonObject().GetObject("object").GetBoolean("nested"), true);

    const infra::JsonStringNavigatorToken stringToken{ "string" };
    const infra::JsonObjectNavigatorToken objectToken{ "object" };
    const infra::JsonBoolNavigatorToken objectNestedToken{ "nested" };

    EXPECT_EQ(jsonFileReader->GetNavigator() / stringToken, "value");
    EXPECT_EQ(jsonFileReader->GetNavigator() / objectToken / objectNestedToken, true);
}

TEST_F(TestJsonFileReader, read_empty_json_throws_exception)
{
    EXPECT_THROW(
        {
            try
            {
                jsonFileReader.Emplace(emptyJson);
            }
            catch (const std::exception& e)
            {
                EXPECT_STREQ("JsonFileReader: Missing JSON data.", e.what());
                throw;
            }
        },
        std::exception);
}

TEST_F(TestJsonFileReader, read_invalid_json_throws_exception)
{
    EXPECT_THROW(
        {
            try
            {
                jsonFileReader.Emplace(invalidJson);
            }
            catch (const std::exception& e)
            {
                EXPECT_STREQ("JsonFileReader: Invalid JSON object.", e.what());
                throw;
            }
        },
        std::exception);
}

TEST_F(TestJsonFileReader, read_invalid_nested_json_throws_exception)
{
    EXPECT_THROW(
        {
            try
            {
                jsonFileReader.Emplace(invalidNestedJson);
            }
            catch (const std::exception& e)
            {
                EXPECT_STREQ("JsonFileReader: Invalid JSON object.", e.what());
                throw;
            }
        },
        std::exception);
}

