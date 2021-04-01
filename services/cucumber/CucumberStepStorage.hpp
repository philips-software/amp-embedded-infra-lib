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

        enum StepMatchResult
        {
            success,
            fail,
            duplicate
        };

    public:
        StepMatchResult& MatchResult();
        void SetMatchResult(StepMatchResult result);
        uint8_t& MatchId();
        void SetMatchId(uint8_t& id);

        CucumberStep& GetStep(uint8_t id);
        void MatchStep(const infra::BoundedString& nameToMatch);
        void AddStep(CucumberStep& step);
        void DeleteStep(CucumberStep& step);
        void ClearStorage();
        bool CompareStepName(CucumberStep& step, const infra::BoundedString& stepName);

    private:
        uint8_t matchId;
        StepMatchResult matchResult;

        infra::IntrusiveList<CucumberStep> stepList;
    };
}

#endif