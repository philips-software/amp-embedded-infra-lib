#ifndef SERVICES_CUCUMBER_STEP_STORAGE_HPP 
#define SERVICES_CUCUMBER_STEP_STORAGE_HPP

#include "services/cucumber/CucumberStep.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/BoundedString.hpp"

namespace services
{
    class StepStorage
    {
    public:
        StepStorage();

        ~StepStorage();

    public:
        CucumberStep* MatchStep(uint8_t id);
        CucumberStep* MatchStep(const infra::BoundedString& nameToMatch);
        void AddStep(const CucumberStep& step);
        bool CompareStepName(CucumberStep& step, const infra::BoundedString& stepName);

        uint8_t nrStepMatches;
    private:
        infra::IntrusiveList<CucumberStep> stepList;
    };
}

#endif