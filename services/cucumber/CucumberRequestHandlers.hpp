#ifndef SERVICES_CUCUMBER_REQUEST_HANDLERS_HPP
#define SERVICES_CUCUMBER_REQUEST_HANDLERS_HPP

namespace services
{
    class CucumberScenarioRequestHandler
    {
    public:
        CucumberScenarioRequestHandler() = default;

        void BeginScenario(){};
        void EndScenario(){};
    };
}

#endif
