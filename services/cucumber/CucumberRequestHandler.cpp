#include "services/cucumber/CucumberRequestHandler.hpp"

namespace services
{
    void CucumberScenarioRequestHandlerDefault::BeginScenario(const infra::Function<void()>& onDone)
    {
        onDone();
    };

    void CucumberScenarioRequestHandlerDefault::TagDiscovered(infra::BoundedConstString tag)
    {}

    void CucumberScenarioRequestHandlerDefault::EndScenario(const infra::Function<void()>& onDone)
    {
        onDone();
    };
}
