#include "services/cucumber/CucumberStepStorage.hpp"

namespace services
{
    CucumberStepStorage& CucumberStepStorage::Instance()
    {
        static CucumberStepStorage instance;
        return instance;
    }

    services::CucumberStepStorage::Match CucumberStepStorage::MatchStep(infra::BoundedConstString stepText)
    {
        Match matchResult;
        uint8_t numberOfMatches = 0;
        uint8_t stepId = 0;

        for (const auto& step : steps)
        {
            if (step.Matches(stepText))
            {
                matchResult.id = stepId;
                numberOfMatches++;
            }
            stepId++;
        }

        if (numberOfMatches == 0)
            matchResult.result = StepMatchResult::Fail;
        else if (numberOfMatches > 1)
            matchResult.result = StepMatchResult::Duplicate;
        else
        {
            matchResult.result = StepMatchResult::Success;
            matchResult.step = &GetStep(matchResult.id);
        }

        return matchResult;
    }

    CucumberStep& CucumberStepStorage::GetStep(uint32_t id)
    {
        really_assert(id < steps.size());
        return *std::next(steps.begin(), id);
    }

    void CucumberStepStorage::AddStep(CucumberStep& step)
    {
        steps.push_back(step);
    }
}
