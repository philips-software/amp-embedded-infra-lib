#include "gmock/gmock.h"
#include "infra/util/BoundedString.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "services/network/CucumberWireProtocolServer.hpp"

class CucumberWireProtocolParserTest
{
};

TEST(TestCucumberWireProtocolParser, should_respond_to_step_matches_with_id_and_arguments_with_success)
{
	services::CucumberWireProtocolParser cucumberWireProtocolParser;
	infra::BoundedString string(R"(["step_matches",{"name_to_match":"we're all wired"}])");
	cucumberWireProtocolParser.ParseRequest(infra::StringAsByteRange(string));
}
