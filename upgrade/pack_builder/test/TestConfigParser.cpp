#include "gmock/gmock.h"
#include "hal/interfaces/test_doubles/FileSystemStub.hpp"
#include "infra/util/BoundedString.hpp"
#include "pack_builder/UpgradePackConfigParser.hpp"

namespace {
    static const std::string completeConfig =
        R"({
            "output_filename": "output/goes/here",
            "components": {
                "boot1st": "wifi_reference.boot_loader_1_prefixed.bin",
                "lut": "APPS.bin",
                "dct": "DCT.bin",
                "filesys": "filesystem.bin",
            },
            "options": {
                "integer_option": 10,
                "string_option": "value"
            }
          })";
}

class TestConfigParser
    : public testing::Test
{
public:
    infra::Optional<infra::JsonObject> configJson;
};

TEST_F(TestConfigParser, parse_config_file_with_invalid_json_throws_exception)
{
    configJson.Emplace(R"( { invalid )");

    try
    {
        application::UpgradePackConfigParser parser(*configJson);
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: invalid JSON"), ex.what());
    }
}

TEST_F(TestConfigParser, parse_config_file_with_missing_required_key_throws_exception)
{
    configJson.Emplace(R"({ "key": "value" })");

    try
    {
        application::UpgradePackConfigParser parser(*configJson);
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: required key components missing"), ex.what());
    }
}

TEST_F(TestConfigParser, GetComponents_returns_empty_component_list_when_components_object_is_empty)
{
    configJson.Emplace(R"({ "components": {} })");
    std::vector<std::pair<std::string, std::string>> emptyComponentsList;

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ(emptyComponentsList, parser.GetComponents());
}

TEST_F(TestConfigParser, non_string_component_value_throws_exception)
{
    configJson.Emplace(R"({ "components": { "invalid component":{} } })");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetComponents();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: invalid value for component: invalid component"), ex.what());
    }
}

TEST_F(TestConfigParser, GetComponents_returns_empty_component_list_when_components_is_not_an_object)
{
    configJson.Emplace(R"({ "components": "value" })");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetComponents();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: components list should be an object"), ex.what());
    }
}

TEST_F(TestConfigParser, GetComponents_returns_component_list_when_components_object_is_not_empty)
{
    configJson.Emplace(infra::BoundedConstString(completeConfig.c_str()));
    std::vector<std::pair<std::string, std::string>> componentsList;
    componentsList.push_back(std::pair<std::string, std::string>("boot1st", "wifi_reference.boot_loader_1_prefixed.bin"));
    componentsList.push_back(std::pair<std::string, std::string>("lut", "APPS.bin"));
    componentsList.push_back(std::pair<std::string, std::string>("dct", "DCT.bin"));
    componentsList.push_back(std::pair<std::string, std::string>("filesys", "filesystem.bin"));

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ(componentsList, parser.GetComponents());
}

TEST_F(TestConfigParser, GetOptions_returns_empty_options_list_when_options_are_not_included)
{
    configJson.Emplace(R"({ "components": {} })");
    std::vector<std::pair<std::string, std::string>> emptyOptionsList;

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ(emptyOptionsList, parser.GetOptions());
}

TEST_F(TestConfigParser, GetOptions_returns_empty_options_list_when_options_object_is_empty)
{
    configJson.Emplace(R"({ "components": {}, "options": {} })");
    std::vector<std::pair<std::string, std::string>> emptyOptionsList;

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ(emptyOptionsList, parser.GetOptions());
}

TEST_F(TestConfigParser, GetOptions_throws_exception_when_options_is_not_an_object)
{
    configJson.Emplace(R"({ "components": {}, "options": "value" })");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetOptions();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: options list should be an object"), ex.what());
    }
}

TEST_F(TestConfigParser, GetOptions_throws_exception_when_all_the_option_values_are_not_strings)
{
    configJson.Emplace(R"({ "components": {}, "options": { "invalid option": {} } })");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetOptions();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: invalid value for option: invalid option"), ex.what());
    }
}

TEST_F(TestConfigParser, GetOptions_returns_options_list_with_all_options)
{
    configJson.Emplace(infra::BoundedConstString(completeConfig.c_str()));
    std::vector<std::pair<std::string, std::string>> optionsList;
    optionsList.push_back(std::pair<std::string, std::string>("integer_option", "10"));
    optionsList.push_back(std::pair<std::string, std::string>("string_option", "value"));

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ(optionsList, parser.GetOptions());
}

TEST_F(TestConfigParser, GetOutputFilename_returns_empty_string_when_output_filename_is_missing)
{
    configJson.Emplace(R"( { "components":{} } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ("", parser.GetOutputFilename());
}

TEST_F(TestConfigParser, GetOutputFilename_throws_exception_when_output_filename_is_not_string)
{
    configJson.Emplace(R"({ "components": {}, "output_filename": {} })");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetOutputFilename();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: output filename should be a string"), ex.what());
    }
}

TEST_F(TestConfigParser, GetOutputFilename_returns_output_filename_as_string)
{
    configJson.Emplace(R"( { "components":{}, "output_filename":"output filename" } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ("output filename", parser.GetOutputFilename());
}