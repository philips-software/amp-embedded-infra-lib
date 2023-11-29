#include "services/cucumber/CucumberRequestHandlers.hpp"

namespace services
{
    void CucumberScenarioRequestHandler::BeginScenario(infra::JsonArray&, const infra::Function<void()>& onDone)
    {
        onDone();
    }

    void CucumberScenarioRequestHandler::EndScenario(const infra::Function<void()>& onDone)
    {
        onDone();
    };
}
