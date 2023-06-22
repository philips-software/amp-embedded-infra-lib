#include "services/cucumber/CucumberStepStorage.hpp"

namespace services
{
    CucumberStepStorage& CucumberStepStorage::Instance()
    {
        static CucumberStepStorage instance;
        return instance;
    }

    services::CucumberStepStorage::StepMatch CucumberStepStorage::MatchStep(infra::BoundedConstString stepText)
    {
        std::size_t numberOfMatches = 0;
        std::size_t stepIndex = 0;
        std::size_t stepId = 0;

        for (const auto& step : steps)
        {
            if (step.Matches(stepText))
            {
                stepId = stepIndex;
                ++numberOfMatches;
            }

            ++stepIndex;
        }

        if (numberOfMatches == 0)
            return { StepMatchResult::notFound, 0, nullptr };
        else if (numberOfMatches > 1)
            return { StepMatchResult::duplicate, 0, nullptr };
        else
            return { StepMatchResult::found, stepId, &GetStep(stepId) };
    }

    CucumberStep& CucumberStepStorage::GetStep(std::size_t id)
    {
        really_assert(id < steps.size());
        return *std::next(steps.begin(), id);
    }

    void CucumberStepStorage::AddStep(CucumberStep& step)
    {
        steps.push_back(step);
    }
}
