#ifndef SERVICES_CUCUMBER_STEP_STORAGE_HPP 
#define SERVICES_CUCUMBER_STEP_STORAGE_HPP

#include "services/cucumber/CucumberStep.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/InterfaceConnector.hpp"

namespace services
{
    class CucumberStepStorage
        : public infra::InterfaceConnector<CucumberStepStorage>
    {
    public:
        CucumberStepStorage();

        ~CucumberStepStorage();

    public:
        CucumberStep* MatchStep(uint8_t id);
        CucumberStep* MatchStep(const infra::BoundedString& nameToMatch);
        void AddStep(CucumberStep& step);
        void DeleteStep(CucumberStep& step);
        void ClearStorage();
        bool CompareStepName(CucumberStep& step, const infra::BoundedString& stepName);

        uint8_t nrStepMatches;
    private:
        infra::IntrusiveList<CucumberStep> stepList;
    };
}

#endif