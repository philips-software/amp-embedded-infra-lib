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

    public:
        infra::Optional<Step> MatchStep(uint8_t id);
        infra::Optional<Step> MatchStep(const infra::BoundedString& nameToMatch);
        void AddStep(const Step& step);
        bool CompareStepName(Step& step, const infra::BoundedString& stepName);

        uint8_t nrStepMatches;
    private:
        infra::IntrusiveList<Step> stepList;
    };
}

#endif