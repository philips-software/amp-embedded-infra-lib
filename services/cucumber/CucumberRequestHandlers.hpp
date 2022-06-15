#ifndef SERVICES_CUCUMBER_REQUEST_HANDLERS_HPP 
#define SERVICES_CUCUMBER_REQUEST_HANDLERS_HPP

#include "infra/util/Function.hpp"

namespace services
{
    class CucumberScenarioRequestHandler
    {
    public:
        CucumberScenarioRequestHandler() = default;
        ~CucumberScenarioRequestHandler() = default;

        virtual void BeginScenario(const infra::Function<void()>& onDone) { onDone(); };
        virtual void EndScenario(const infra::Function<void()>& onDone) { onDone(); };
    };
}

#endif
