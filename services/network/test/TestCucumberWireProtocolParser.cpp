#include "gmock/gmock.h"
#include "infra/util/BoundedString.hpp"
#include "infra/syntax/Json.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "infra/stream/StringOutputStream.hpp"

class WireProtocol
{
	public:
		enum StepMatchResult
		{
			SUCCESS,
			FAILED
		};

		static infra::BoundedString::WithStorage<128> ParseJsonArray(infra::JsonArray& input);
		static infra::BoundedString::WithStorage<20> SuccessMessage();
		static infra::BoundedString::WithStorage<128> FailureMessage(infra::BoundedConstString::WithStorage<64> failMessage, infra::BoundedConstString::WithStorage<64> exceptionType);

};

infra::BoundedString::WithStorage<128> WireProtocol::ParseJsonArray(infra::JsonArray& input)
{
	infra::JsonArrayIterator iterator(input.begin());
	infra::BoundedString::WithStorage<16> requestMessage;
	iterator->Get<infra::JsonString>().ToString(requestMessage);

	if (requestMessage == "step_matches")
	{
		// Get name to match from Json array
		iterator++;
		infra::JsonObject stepObject = iterator->Get<infra::JsonObject>();
		infra::BoundedString::WithStorage<256> nameToMatch;
		stepObject.GetString("name_to_match").ToString(nameToMatch);

		// Find matching ID and arguments
		uint8_t id = 1;
		infra::JsonArray argumentArray("[]");
		
		// If no ID found, result = failed
		StepMatchResult stepMatchResult= SUCCESS;

		// Create Json Array with result
		infra::BoundedString::WithStorage<64> resultString;
		{
			infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, resultString);

			if (stepMatchResult == SUCCESS)
			{
				result.Add((infra::BoundedConstString) "success");
				infra::JsonArrayFormatter subArray(result.SubArray());

				infra::JsonObjectFormatter subObject(subArray.SubObject());
				infra::StringOutputStream::WithStorage<10> idStream;
				idStream << id;
				subObject.Add("id", idStream.Storage());

				subObject.Add(infra::JsonKeyValue{ "args", infra::JsonValue(infra::InPlaceType<infra::JsonArray>(), argumentArray) });
			}
		}

		// Add newline to end Json message
		resultString.insert(resultString.size(), "\n");

		return resultString;
	}
	else if (requestMessage == "begin_scenario")
	{
		return SuccessMessage();
	}
	else if (requestMessage == "end_scenario")
	{
		return SuccessMessage();
	}
	else if (requestMessage == "invoke")
	{
		// Find invoke's ID and arguments
		iterator++;
		infra::BoundedString::WithStorage<32> idString;
		iterator->Get<infra::JsonObject>().GetString("id").ToString(idString);

		infra::BoundedString::WithStorage<512> argsString;
		infra::JsonArray subArray = iterator->Get<infra::JsonObject>().GetArray("args");
		if (subArray.begin() != subArray.end())
		{
			infra::JsonArrayIterator subIterator(subArray.begin());
			subIterator->Get<infra::JsonString>().ToString(argsString);
		}

		// Find according response
		infra::BoundedString::WithStorage<32> responseHeader = "pending";
		infra::BoundedString::WithStorage<32> responseMessage = "I'll do it later";

		// Create response JSON Array
		infra::BoundedString::WithStorage<128> resultString;
		{
			infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, resultString);
			result.Add(responseHeader);
			result.Add(responseMessage);
		}
		// Add newline to end Json message
		resultString.insert(resultString.size(), "\n");

		return resultString;
	}
	else if (requestMessage == "snippet_text")
	{
		return SuccessMessage();
	}
	else
	{
		return FailureMessage("Invalid Request", "Some.Foreign.ExceptionType");
	}
}

infra::BoundedString::WithStorage<128> WireProtocol::FailureMessage(infra::BoundedConstString::WithStorage<64> failMessage, infra::BoundedConstString::WithStorage<64> exceptionType)
{
	infra::BoundedString::WithStorage<128> resultString;
	{
		infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, resultString);
		result.Add((infra::BoundedConstString) "fail");

		infra::JsonObjectFormatter subObject(result.SubObject());
		subObject.Add("message", failMessage);
		subObject.Add("exception", exceptionType);
	}
	// Add newline to end Json message
	resultString.insert(resultString.size(), "\n");

	return resultString;
}

infra::BoundedString::WithStorage<20> WireProtocol::SuccessMessage()
{
	infra::BoundedString::WithStorage<20> resultString;
	{
		infra::JsonArrayFormatter::WithStringStream result(infra::inPlace, resultString);
		result.Add((infra::BoundedConstString) "success");
		infra::JsonArrayFormatter subArray(result.SubArray());
	}
	// Add newline to end Json message
	resultString.insert(resultString.size(), "\n");

	return resultString;
}

TEST(TestCucumberWireProtocolParser, TestFailureMessage)
{
	EXPECT_EQ(0, WireProtocol::FailureMessage("Invalid Request", "Some.Foreign.ExceptionType").compare("[ \"fail\", { \"message\":\"Invalid Request\", \"exception\":\"Some.Foreign.ExceptionType\" } ]\n"));
}

TEST(TestCucumberWireProtocolParser, TestSuccessMessage)
{
	EXPECT_EQ(0, WireProtocol::SuccessMessage().compare("[ \"success\", [  ] ]\n"));
}

TEST(TestCucumberWireProtocolParser, should_respond_to_invoke_with_pending)
{
	infra::JsonArray jsonArray(R"(["invoke",{"id":"1","args":[]}])");
	EXPECT_EQ(0, WireProtocol::ParseJsonArray(jsonArray).compare("[ \"pending\", \"I'll do it later\" ]\n"));
}

TEST(TestCucumberWireProtocolParser, should_respond_to_step_matches_with_id_and_arguments_with_success)
{
	infra::JsonArray jsonArray(R"(["step_matches",{"name_to_match":"we're all wired"}])");
	EXPECT_EQ(0, WireProtocol::ParseJsonArray(jsonArray).compare("[ \"success\", [ { \"id\":\"1\", \"args\":[] } ] ]\n"));
}

TEST(TestCucumberWireProtocolParser, should_respond_to_begin_scenario_with_success)
{
	infra::JsonArray jsonArray(R"(["begin_scenario"])");
	EXPECT_EQ(0, WireProtocol::ParseJsonArray(jsonArray).compare("[ \"success\", [  ] ]\n"));
}

TEST(TestCucumberWireProtocolParser, should_respond_to_end_scenario_with_success)
{
	infra::JsonArray jsonArray(R"(["end_scenario"])");
	EXPECT_EQ(0, WireProtocol::ParseJsonArray(jsonArray).compare("[ \"success\", [  ] ]\n"));
}

TEST(TestCucumberWireProtocolParser, should_respond_to_invalid_request_with_fail_message)
{
	infra::JsonArray jsonArray(R"(["invalid request"])");
	EXPECT_EQ(0, WireProtocol::ParseJsonArray(jsonArray).compare("[ \"fail\", { \"message\":\"Invalid Request\", \"exception\":\"Some.Foreign.ExceptionType\" } ]\n"));
}
