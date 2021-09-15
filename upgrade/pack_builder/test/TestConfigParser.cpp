#include "hal/interfaces/test_doubles/FileSystemStub.hpp"
#include "infra/util/BoundedString.hpp"
#include "pack_builder/UpgradePackConfigParser.hpp"
#include "gmock/gmock.h"
#include <climits>

namespace
{
    static const std::string completeConfig =
        R"({
            "output_filename": "output/goes/here",
            "components": {
                "boot1st": "first_stage_bootloader.bin",
                "lut": "look_up_table.bin",
                "filesys": "filesystem.bin"
            },
            "upgrade_configuration": {
                "product_name": "product name",
                "product_version": "product version",
                "component_name": "component name",
                "component_version": 1
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
    EXPECT_EQ(emptyComponentsList, parser.GetComponents());
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
    componentsList.push_back(std::pair<std::string, std::string>("boot1st", "first_stage_bootloader.bin"));
    componentsList.push_back(std::pair<std::string, std::string>("lut", "look_up_table.bin"));
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

TEST_F(TestConfigParser, GetUpgradeKeys_returns_empty_string_when_upgrade_keys_is_missing)
{
    configJson.Emplace(R"( { "components":{} } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ("", parser.GetUpgradeKeys());
}

TEST_F(TestConfigParser, GetUpgradeKeys_throws_exception_when_upgrade_keys_is_not_string)
{
    configJson.Emplace(R"({ "components": {}, "upgrade_keys": {} })");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetUpgradeKeys();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: upgrade_keys should be a string"), ex.what());
    }
}

TEST_F(TestConfigParser, GetUpgradeKeys_returns_upgrade_keys_as_string)
{
    configJson.Emplace(R"( { "components":{}, "upgrade_keys":"upgrade_keys.bin" } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ("upgrade_keys.bin", parser.GetUpgradeKeys());
}

TEST_F(TestConfigParser, GetUpgradeConfiguration_throws_exception_when_upgrade_configuration_is_missing)
{
    configJson.Emplace(R"( { "components":{} } )");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetUpgradeConfiguration();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: requested key upgrade_configuration is missing"), ex.what());
    }
}

TEST_F(TestConfigParser, GetUpgradeConfiguration_throws_exception_when_upgrade_configuration_is_not_object)
{
    configJson.Emplace(R"({ "components": {}, "upgrade_configuration": "not an object" })");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetUpgradeConfiguration();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: upgrade_configuration should be an object"), ex.what());
    }
}

TEST_F(TestConfigParser, GetUpgradeConfiguration_returns_upgrade_configuration_as_object)
{
    configJson.Emplace(R"( { "components":{}, "upgrade_configuration": { "key":"value" } } )");

    infra::JsonObject upgradeConfigurationJson(R"( { "key":"value" } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ(upgradeConfigurationJson, parser.GetUpgradeConfiguration());
}

TEST_F(TestConfigParser, GetProductName_throws_exception_when_upgrade_configuration_product_name_is_missing)
{
    configJson.Emplace(R"( { "components":{} } )");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetProductName();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: requested key upgrade_configuration is missing"), ex.what());
    }
}

TEST_F(TestConfigParser, GetProductName_returns_empty_string_when_product_name_is_missing)
{
    configJson.Emplace(R"( { "components":{}, "upgrade_configuration":{} } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ("", parser.GetProductName());
}

TEST_F(TestConfigParser, GetProductName_throws_exception_when_product_name_is_not_string)
{
    configJson.Emplace(R"({ "components": {}, "upgrade_configuration":{ "product_name": {} } })");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetProductName();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: upgrade_configuration/product_name should be a string"), ex.what());
    }
}

TEST_F(TestConfigParser, GetProductName_returns_product_name_as_string)
{
    configJson.Emplace(R"( { "components":{}, "upgrade_configuration":{ "product_name": "product_name.bin" } } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ("product_name.bin", parser.GetProductName());
}

TEST_F(TestConfigParser, GetProductVersion_throws_exception_when_upgrade_configuration_product_version_is_missing)
{
    configJson.Emplace(R"( { "components":{} } )");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetProductVersion();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: requested key upgrade_configuration is missing"), ex.what());
    }
}

TEST_F(TestConfigParser, GetProductVersion_returns_empty_string_when_product_version_is_missing)
{
    configJson.Emplace(R"( { "components":{}, "upgrade_configuration":{} } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ("", parser.GetProductVersion());
}

TEST_F(TestConfigParser, GetProductVersion_throws_exception_when_product_version_is_not_string)
{
    configJson.Emplace(R"({ "components": {}, "upgrade_configuration":{ "product_version": {} } })");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetProductVersion();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: upgrade_configuration/product_version should be a string"), ex.what());
    }
}

TEST_F(TestConfigParser, GetProductVersion_returns_product_version_as_string)
{
    configJson.Emplace(R"( { "components":{}, "upgrade_configuration":{ "product_version": "product_version.bin" } } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ("product_version.bin", parser.GetProductVersion());
}

TEST_F(TestConfigParser, GetComponentName_throws_exception_when_upgrade_configuration_component_name_is_missing)
{
    configJson.Emplace(R"( { "components":{} } )");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetComponentName();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: requested key upgrade_configuration is missing"), ex.what());
    }
}

TEST_F(TestConfigParser, GetComponentName_returns_empty_string_when_component_name_is_missing)
{
    configJson.Emplace(R"( { "components":{}, "upgrade_configuration":{} } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ("", parser.GetComponentName());
}

TEST_F(TestConfigParser, GetComponentName_throws_exception_when_component_name_is_not_string)
{
    configJson.Emplace(R"({ "components": {}, "upgrade_configuration":{ "component_name": {} } })");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetComponentName();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: upgrade_configuration/component_name should be a string"), ex.what());
    }
}

TEST_F(TestConfigParser, GetComponentName_returns_component_name_as_string)
{
    configJson.Emplace(R"( { "components":{}, "upgrade_configuration":{ "component_name": "component_name.bin" } } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ("component_name.bin", parser.GetComponentName());
}

TEST_F(TestConfigParser, GetComponentVersion_throws_exception_when_upgrade_configuration_component_version_is_missing)
{
    configJson.Emplace(R"( { "components":{} } )");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetComponentVersion();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: requested key upgrade_configuration is missing"), ex.what());
    }
}

TEST_F(TestConfigParser, GetComponentVersion_returns_negative_1_when_component_version_is_missing)
{
    configJson.Emplace(R"( { "components":{}, "upgrade_configuration":{} } )");

    application::UpgradePackConfigParser parser(*configJson);
    uint32_t negativeOne = std::numeric_limits<uint32_t>::max();
    ASSERT_EQ(negativeOne, parser.GetComponentVersion());
}

TEST_F(TestConfigParser, GetComponentVersion_throws_exception_when_component_version_is_not_integer)
{
    configJson.Emplace(R"({ "components": {}, "upgrade_configuration":{ "component_version": {} } })");
    application::UpgradePackConfigParser parser(*configJson);

    try
    {
        parser.GetComponentVersion();
        FAIL() << "Expected ConfigParser::ParseException";
    }
    catch (const application::UpgradePackConfigParser::ParseException& ex)
    {
        EXPECT_EQ(std::string("ConfigParser error: upgrade_configuration/component_version should be an integer"), ex.what());
    }
}

TEST_F(TestConfigParser, GetComponentVersion_returns_component_version_as_integer)
{
    configJson.Emplace(R"( { "components":{}, "upgrade_configuration":{ "component_version": 1 } } )");

    application::UpgradePackConfigParser parser(*configJson);
    ASSERT_EQ(1, parser.GetComponentVersion());
}
