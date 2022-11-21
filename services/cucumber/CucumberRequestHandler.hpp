#ifndef SERVICES_CUCUMBER_REQUEST_HANDLERS_HPP
#define SERVICES_CUCUMBER_REQUEST_HANDLERS_HPP

#include "infra/util/BoundedString.hpp"
#include "infra/util/Function.hpp"

namespace services
{
    class CucumberScenarioRequestHandler
    {
    public:
        virtual void BeginScenario(const infra::Function<void()>& onDone) = 0;
        virtual void EndScenario(const infra::Function<void()>& onDone) = 0;
        virtual void TagDiscovered(infra::BoundedConstString tag) = 0;
    };

    class CucumberScenarioRequestHandlerDefault
        : public CucumberScenarioRequestHandler
    {
    public:
        void BeginScenario(const infra::Function<void()>& onDone) override;
        void EndScenario(const infra::Function<void()>& onDone) override;
        void TagDiscovered(infra::BoundedConstString tag) override;
    };
}

#endif
