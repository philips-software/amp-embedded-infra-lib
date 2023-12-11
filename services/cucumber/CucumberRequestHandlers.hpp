#ifndef SERVICES_CUCUMBER_REQUEST_HANDLERS_HPP
#define SERVICES_CUCUMBER_REQUEST_HANDLERS_HPP

#include "infra/syntax/Json.hpp"
#include "infra/util/Function.hpp"

namespace services
{
    class CucumberScenarioRequestHandler
    {
    public:
        virtual ~CucumberScenarioRequestHandler() = default;

        virtual void BeginScenario(infra::JsonArray& tags, const infra::Function<void()>& onDone);
        virtual void EndScenario(const infra::Function<void()>& onDone);
    };
}

#endif
