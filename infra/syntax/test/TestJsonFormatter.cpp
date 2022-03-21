#include "gtest/gtest.h"
#include "infra/syntax/JsonFormatter.hpp"

TEST(BasicUsageTest, format_json_object)
{
    infra::BoundedString::WithStorage<100> response;
    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, response);
        formatter.Add("name", "Upgrade 19.2");
        formatter.Add("version", "19.2");
        formatter.Add("canupgrade", true);
    }

    EXPECT_EQ(R"({ "name":"Upgrade 19.2", "version":"19.2", "canupgrade":true })", response);
}

TEST(JsonObjectFormatter, construction_results_in_empty_object)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
    }

    EXPECT_EQ("{  }", string);
}

TEST(JsonObjectFormatter, add_bool)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add("trueTag", true);
        formatter.Add("falseTag", false);
    }

    EXPECT_EQ(R"({ "trueTag":true, "falseTag":false })", string);
}

TEST(JsonObjectFormatter, add_int)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add("intTag", 0);
        formatter.Add("uint32Tag", static_cast<uint32_t>(5));
        formatter.Add("int64Tag", static_cast<int64_t>(-10));
    }

    EXPECT_EQ(R"({ "intTag":0, "uint32Tag":5, "int64Tag":-10 })", string);
}

TEST(JsonObjectFormatter, add_const_char_ptr)
{
    infra::BoundedString::WithStorage<64> string;

    {
        const char* s = "test";
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add("tag", s);
    }

    EXPECT_EQ(R"({ "tag":"test" })", string);
}

TEST(JsonObjectFormatter, add_BoundedConstString)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::BoundedConstString value("test");
        infra::BoundedConstString key("tag");
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add("tag", value);
        formatter.Add(key, value);
    }

    EXPECT_EQ(R"({ "tag":"test", "tag":"test" })", string);
}

TEST(JsonObjectFormatter, add_milli_float)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.AddMilliFloat("tag", 12, 34);
    }

    EXPECT_EQ(R"({ "tag":12.034 })", string);
}

TEST(JsonObjectFormatter, add_sting_as_sub_object)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::BoundedConstString s(R"({ "test": 123 })");
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.AddSubObject("tag", s);
    }

    EXPECT_EQ(R"({ "tag":{ "test": 123 } })", string);
}

TEST(JsonObjectFormatter, add_sub_object)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        {
            infra::JsonObjectFormatter subObject(formatter.SubObject("tag"));
            subObject.Add("subTagName", "value");
        }
    }

    EXPECT_EQ(R"({ "tag":{ "subTagName":"value" } })", string);
}

TEST(JsonObjectFormatter, add_sub_array)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        {
            infra::JsonArrayFormatter subArray(formatter.SubArray("tag"));
            subArray.Add("value");
        }
    }

    EXPECT_EQ(R"({ "tag":[ "value" ] })", string);
}

TEST(JsonObjectFormatter, add_string_via_stream)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::BoundedConstString s("test");
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        auto stream = formatter.AddString("tag");
        stream << s;
    }

    EXPECT_EQ(R"({ "tag":"test" })", string);
}

TEST(JsonObjectFormatter, add_object_via_stream)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::BoundedConstString s(R"({ [ "test" ] })");
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        auto stream = formatter.AddObject("tag");
        stream << s;
    }

    EXPECT_EQ(R"({ "tag":{ [ "test" ] } })", string);
}

TEST(JsonObjectFormatter, add_json_value_bool)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add(infra::JsonKeyValue{ "tag", infra::JsonValue(infra::InPlaceType<bool>(), true) });
    }

    EXPECT_EQ(R"({ "tag":true })", string);
}

TEST(JsonObjectFormatter, add_json_value_int)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add(infra::JsonKeyValue{ "tag", infra::JsonValue(infra::InPlaceType<int32_t>(), 5) });
    }

    EXPECT_EQ(R"({ "tag":5 })", string);
}

TEST(JsonObjectFormatter, add_json_value_string)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add(infra::JsonKeyValue{ "tag\\\"", infra::JsonValue(infra::InPlaceType<infra::JsonString>(), R"(ab\"cd)") });
    }

    EXPECT_EQ("{ \"tag\\\"\":\"ab\\\"cd\" }", string);
}

TEST(JsonObjectFormatter, add_json_value_object)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add(infra::JsonKeyValue{ "tag", infra::JsonValue(infra::InPlaceType<infra::JsonObject>(), infra::JsonObject("{}")) });
    }

    EXPECT_EQ(R"({ "tag":{} })", string);
}

TEST(JsonObjectFormatter, add_json_value_array)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add(infra::JsonKeyValue{ "tag", infra::JsonValue(infra::InPlaceType<infra::JsonArray>(), infra::JsonArray("[]")) });
    }

    EXPECT_EQ(R"({ "tag":[] })", string);
}

TEST(JsonObjectFormatter, output_is_truncated_on_small_output_string)
{
    infra::BoundedString::WithStorage<1> string;

    {
        infra::BoundedConstString s("test");
        infra::StringOutputStream stream(string, infra::noFail);
        infra::JsonObjectFormatter formatter(stream);
        formatter.Add("tag", s);
    }

    EXPECT_EQ(R"({)", string);
}

TEST(JsonObjectFormatter, move_object_formatter)
{
    infra::BoundedString::WithStorage<100> string;

    {
        auto&& formatter = infra::JsonObjectFormatter::WithStringStream(infra::inPlace, string);
        auto subObject = std::move(formatter.SubObject("tag"));
    }

    EXPECT_EQ(R"({ "tag":{  } })", string);
}

TEST(JsonObjectFormatter, move_array_formatter)
{
    infra::BoundedString::WithStorage<100> string;

    {
        auto&& formatter = infra::JsonObjectFormatter::WithStringStream(infra::inPlace, string);
        auto subArray = std::move(formatter.SubArray("tag"));
    }

    EXPECT_EQ(R"({ "tag":[  ] })", string);
}

TEST(JsonObjectFormatter, escape_strings)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add("tag", "\"\b\f\n\r\t\x11");
    }

    EXPECT_EQ("{ \"tag\":\"\\\"\\b\\f\\n\\r\\t\\u0011\" }", string);
}

TEST(JsonObjectFormatter, JsonEscapedStringSize_results_in_size_of_escaped_string)
{
    EXPECT_EQ(0, infra::JsonEscapedStringSize(""));
    EXPECT_EQ(2, infra::JsonEscapedStringSize("{}"));
    EXPECT_EQ(2, infra::JsonEscapedStringSize("\""));
    EXPECT_EQ(4, infra::JsonEscapedStringSize("\r\n"));
    EXPECT_EQ(12, infra::JsonEscapedStringSize("\x11\x11"));
}

TEST(JsonObjectFormatter, JsonSubStringOfMaxEscapedSize_returns_substring_of_the_correct_size)
{
    EXPECT_EQ("", infra::JsonSubStringOfMaxEscapedSize("a\n\x11 ", 0));
    EXPECT_EQ("a", infra::JsonSubStringOfMaxEscapedSize("a\n\x11 ", 1));
    EXPECT_EQ("a", infra::JsonSubStringOfMaxEscapedSize("a\n\x11 ", 2));
    EXPECT_EQ("a\n", infra::JsonSubStringOfMaxEscapedSize("a\n\x11 ", 3));
    EXPECT_EQ("a\n", infra::JsonSubStringOfMaxEscapedSize("a\n\x11 ", 8));
    EXPECT_EQ("a\n\x11", infra::JsonSubStringOfMaxEscapedSize("a\n\x11 ", 9));
    EXPECT_EQ("a\n\x11 ", infra::JsonSubStringOfMaxEscapedSize("a\n\x11 ", 10));
    EXPECT_EQ("a\n\x11 ", infra::JsonSubStringOfMaxEscapedSize("a\n\x11 ", 11));
}

TEST(JsonArrayFormatter, construction_results_in_empty_object)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonArrayFormatter::WithStringStream formatter(infra::inPlace, string);
    }

    EXPECT_EQ("[  ]", string);
}

TEST(JsonArrayFormatter, add_bool)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonArrayFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add(true);
        formatter.Add(false);
    }

    EXPECT_EQ(R"([ true, false ])", string);
}

TEST(JsonArrayFormatter, add_int)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonArrayFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add(0);
        formatter.Add(static_cast<uint32_t>(5));
        formatter.Add(static_cast<int64_t>(-10));
    }

    EXPECT_EQ(R"([ 0, 5, -10 ])", string);
}

TEST(JsonArrayFormatter, add_const_char_ptr)
{
    infra::BoundedString::WithStorage<64> string;

    {
        const char* s = "test";
        infra::JsonArrayFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add(s);
    }

    EXPECT_EQ(R"([ "test" ])", string);
}

TEST(JsonArrayFormatter, add_BoundedConstString)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::BoundedConstString s("test");
        infra::JsonArrayFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Add(s);
    }

    EXPECT_EQ(R"([ "test" ])", string);
}

TEST(JsonArrayFormatter, add_sub_object)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonArrayFormatter::WithStringStream formatter(infra::inPlace, string);
        {
            infra::JsonObjectFormatter subObject(formatter.SubObject());
            subObject.Add("subTagName", "value");
        }
    }

    EXPECT_EQ(R"([ { "subTagName":"value" } ])", string);
}

TEST(JsonArrayFormatter, add_sub_array)
{
    infra::BoundedString::WithStorage<64> string;

    {
        infra::JsonArrayFormatter::WithStringStream formatter(infra::inPlace, string);
        {
            infra::JsonArrayFormatter subArray(formatter.SubArray());
            subArray.Add("value");
        }
    }

    EXPECT_EQ(R"([ [ "value" ] ])", string);
}

TEST(JsonArrayFormatter, output_is_truncated_on_small_output_string)
{
    infra::BoundedString::WithStorage<1> string;

    {
        infra::BoundedConstString s("test");
        infra::StringOutputStream stream(string, infra::noFail);
        infra::JsonArrayFormatter formatter(stream);
        formatter.Add(s);
    }

    EXPECT_EQ(R"([)", string);
}

TEST(JsonObjectFormatter, merge_single_key_int_value_pair)
{
    infra::BoundedConstString jsonPath = "path1";
    infra::JsonValue jsonValue = 5;
    std::string mainJsonConfig = R"({"path1":6})";
    infra::JsonObject jsonConfigObj{ mainJsonConfig };
    infra::BoundedString::WithStorage<64> string;
    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        infra::Merge(formatter, jsonConfigObj, jsonPath, jsonValue);
    }
    
    EXPECT_EQ(R"({ "path1":5 })", string);
}

TEST(JsonObjectFormatter, merge_single_key_string_value_pair)
{
    infra::BoundedConstString jsonPath = "path1";
    infra::JsonValue jsonValue{ infra::JsonString("string") };
    std::string mainJsonConfig = R"({"path1":6})";
    infra::JsonObject jsonConfigObj{ mainJsonConfig };
    infra::BoundedString::WithStorage<64> string;
    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        infra::Merge(formatter, jsonConfigObj, jsonPath, jsonValue);
    }

    EXPECT_EQ(R"({ "path1":"string" })", string);
}

TEST(JsonObjectFormatter, merge_missing_key_string_value_pair)
{
    infra::BoundedConstString jsonPath = "path1/path3";
    infra::JsonValue jsonValue{ infra::JsonString("string") };
    std::string mainJsonConfig = R"({"path1":{"path2" : 5}})";
    infra::JsonObject jsonConfigObj{ mainJsonConfig };
    infra::BoundedString::WithStorage<64> string;
    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        infra::Merge(formatter, jsonConfigObj, jsonPath, jsonValue);
    }

    EXPECT_EQ(R"({ "path1":{ "path2":5, "path3":"string" } })", string);
}

TEST(JsonObjectFormatter, merge_key_empty_value_pair)
{
    infra::BoundedConstString jsonPath = "path1";
    infra::JsonValue jsonValue{ infra::JsonObject("{}") };
    std::string mainJsonConfig = R"({"path1":{"path2" : 5, "path3": "string"}})";
    infra::JsonObject jsonConfigObj{ mainJsonConfig };
    infra::BoundedString::WithStorage<64> string;
    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        infra::Merge(formatter, jsonConfigObj, jsonPath, jsonValue);
    }

    EXPECT_EQ(R"({ "path1":{} })", string);
}

TEST(JsonObjectFormatter, merge_key_string_value_in_max_depth_5)
{
    infra::BoundedConstString jsonPath = "path1/path2/path3/path4/path5";
    infra::JsonValue jsonValue{ infra::JsonString("strings") };
    std::string mainJsonConfig = R"({ "path1":{ "path2":{ "path3":{ "path4":{ "path5":"string"}}}}})";
    infra::JsonObject jsonConfigObj{ mainJsonConfig };
    infra::BoundedString::WithStorage<128> string;
    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        infra::Merge(formatter, jsonConfigObj, jsonPath, jsonValue);
    }

    EXPECT_EQ(R"({ "path1":{ "path2":{ "path3":{ "path4":{ "path5":"strings" } } } } })", string);
}

TEST(JsonObjectFormatter, merge_key_value_with_nested_depth_3)
{
    infra::BoundedConstString jsonPath = "customization/static_configuration/plugin_manager_config";
    infra::JsonValue jsonObj = infra::JsonObject("{\"max_retries\": 2,\"backoff_mins_max_retries_failed\" : 30,\"retry_attach_timeout_mins\" : 1,\"timeout_detach_mins\" : 2}");
    std::string jsonConfig = "{ \"version\": 2, \"image_components\": { \"application\": \"Install/components/stcellular.cellular_cn_sequans_develop_low_power/stm32f767-Release/bin/stcellular.cellular_cn_sequans_develop_low_power.elf\" }, \"customization\": {"
        "\"static_configuration\": { \"app_version\": \"v2.0\", \"power_manager_config\":{ \"power_state_after_timeout\" : \"poweroff\", \"timeout_low_power_mins\" : 1 }, \"plugin_manager_config\": { \"max_retries\": 2, \"backoff_mins_max_retries_failed\" : 30,"
		"\"retry_attach_timeout_mins\" : 1, \"timeout_detach_mins\" : 1 }, \"certificate_connect\" : \"Source/ConnectivityToolKit/hsdp_connect/ccn-ca-certificates-connect.pem\", \"certificate_data\" : \"Source/ConnectivityToolKit/hsdp_connect/ccn-ca-certificates-data.pem\","
		"\"hsdp_connect\": { \"regions\": [ { \"name\": \"EU\", \"iam_url\": \"iam-client-test.eu-west.philips-healthsuite.com\", \"discovery_url\": \"discovery-client-test.eu01.connect.hsdp.io/client-test/core/discovery\", \"ca_certificates\": \"Source/ConnectivityToolKit/hsdp_connect/ccn-ca-certificates.pem\","
		"\"bootstrap_credentials\": { \"username\": \"7671746a4d3849f4aea\", \"password\": \"r2!S7y0qr.G3Hw-K\" } } ], \"device_group_name\": \"DeviceGroupName\", \"device_type_name\": \"OculusDev01\", \"hsdp_iam_proxy_enabled\": false, \"device_purpose\": \"dev\"} }	} }";

    infra::BoundedString::WithStorage<3000> string;
    infra::JsonObject jsonConfigObj{ jsonConfig };
    {
        infra::JsonObjectFormatter::WithStringStream formatter(infra::inPlace, string);
        infra::Merge(formatter, jsonConfigObj, jsonPath, jsonObj);
    }
    infra::JsonObject jsonObjectModified{ string };
    std::string jsonConfigFinal = "{ \"version\": 2, \"image_components\": { \"application\": \"Install/components/stcellular.cellular_cn_sequans_develop_low_power/stm32f767-Release/bin/stcellular.cellular_cn_sequans_develop_low_power.elf\" }, \"customization\": {"
                             "\"static_configuration\": { \"app_version\": \"v2.0\", \"power_manager_config\":{ \"power_state_after_timeout\" : \"poweroff\", \"timeout_low_power_mins\" : 1 }, \"plugin_manager_config\": { \"max_retries\": 2, \"backoff_mins_max_retries_failed\" : 30,"
                             "\"retry_attach_timeout_mins\" : 1, \"timeout_detach_mins\" : 2 }, \"certificate_connect\" : \"Source/ConnectivityToolKit/hsdp_connect/ccn-ca-certificates-connect.pem\", \"certificate_data\" : \"Source/ConnectivityToolKit/hsdp_connect/ccn-ca-certificates-data.pem\","
                             "\"hsdp_connect\": { \"regions\": [ { \"name\": \"EU\", \"iam_url\": \"iam-client-test.eu-west.philips-healthsuite.com\", \"discovery_url\": \"discovery-client-test.eu01.connect.hsdp.io/client-test/core/discovery\", \"ca_certificates\": \"Source/ConnectivityToolKit/hsdp_connect/ccn-ca-certificates.pem\","
                             "\"bootstrap_credentials\": { \"username\": \"7671746a4d3849f4aea\", \"password\": \"r2!S7y0qr.G3Hw-K\" } } ], \"device_group_name\": \"DeviceGroupName\", \"device_type_name\": \"OculusDev01\", \"hsdp_iam_proxy_enabled\": false, \"device_purpose\": \"dev\" } } } }";
    infra::JsonObject jsonObjectFinal{ jsonConfigFinal };
    EXPECT_EQ(jsonObjectModified, jsonObjectFinal);
}
