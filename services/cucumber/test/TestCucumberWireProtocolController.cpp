#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/BoundedString.hpp"
#include "services/cucumber/CucumberContext.hpp"
#include "services/cucumber/CucumberRequestHandlers.hpp"
#include "services/cucumber/CucumberWireProtocolController.hpp"
#include "services/cucumber/CucumberWireProtocolParser.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "gmock/gmock.h"
#include <algorithm>

namespace
{
    class CucumberScenarioRequestHandlerMock
        : public services::CucumberScenarioRequestHandler
    {
    public:
        MOCK_METHOD(void, BeginScenario, (infra::JsonArray & tags, const infra::Function<void()>& onDone), (override));
        MOCK_METHOD(void, EndScenario, (const infra::Function<void()>& onDone), (override));
    };

    class CucumberWireProtocolControllerTest
        : public testing::Test
        , public infra::ClockFixture
    {
    public:
        void Request(infra::BoundedConstString request);
        void RequestWithoutScenarioTags(infra::BoundedConstString request);

    public:
        services::CucumberContext context;
        testing::StrictMock<services::ConnectionObserverMock> connectionObserver;
        testing::StrictMock<CucumberScenarioRequestHandlerMock> scenarioRequestHandler;
        services::CucumberWireProtocolController controller{ connectionObserver, scenarioRequestHandler };
        services::CucumberWireProtocolParser parser;
    };

    void CucumberWireProtocolControllerTest::Request(infra::BoundedConstString request)
    {
        parser.ParseRequest(request);
        controller.HandleRequest(parser);
    }

    void CucumberWireProtocolControllerTest::RequestWithoutScenarioTags(infra::BoundedConstString request)
    {
        parser.ParseRequest(request);
        parser.scenarioTags = infra::none;
        controller.HandleRequest(parser);
    }
};

TEST_F(CucumberWireProtocolControllerTest, begin_scenario_with_uninitialized_tags_calls_with_empty_json_array)
{
    EXPECT_CALL(scenarioRequestHandler, BeginScenario(testing::Eq(infra::JsonArray()), testing::_));
    Request(R"(["begin_scenario"])");
}

TEST_F(CucumberWireProtocolControllerTest, begin_scenario_without_tags_calls_with_empty_json_array)
{
    EXPECT_CALL(scenarioRequestHandler, BeginScenario(testing::Eq(infra::JsonArray()), testing::_));
    RequestWithoutScenarioTags(R"(["begin_scenario"])");
}

TEST_F(CucumberWireProtocolControllerTest, begin_scenario_with_tags_forward_tags_as_json_array)
{
    EXPECT_CALL(scenarioRequestHandler, BeginScenario(testing::Eq(infra::JsonArray(R"(["tag1","tag2"])")), testing::_));
    Request(R"(["begin_scenario",{"tags":["tag1","tag2"]}])");
}
