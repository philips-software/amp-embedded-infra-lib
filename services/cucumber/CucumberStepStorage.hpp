#ifndef SERVICES_CUCUMBER_STEP_STORAGE_HPP
#define SERVICES_CUCUMBER_STEP_STORAGE_HPP

#include "infra/util/BoundedString.hpp"
#include "services/cucumber/CucumberStep.hpp"

namespace services
{
    class CucumberStepStorage
    {
    public:
        CucumberStepStorage() = default;
        CucumberStepStorage& operator=(const CucumberStepStorage& other) = delete;
        CucumberStepStorage(CucumberStepStorage& other) = delete;
        virtual ~CucumberStepStorage() = default;

        enum class StepMatchResult
        {
            Success,
            Fail,
            Duplicate
        };

        struct Match
        {
            StepMatchResult result;
            uint32_t id;
            CucumberStep* step;
        };

        static CucumberStepStorage& Instance();

        bool MatchesStepName(CucumberStep& step, infra::BoundedConstString stepName);
        services::CucumberStepStorage::Match MatchStep(infra::BoundedConstString nameToMatch);

        CucumberStep& GetStep(uint32_t id);
        void AddStep(CucumberStep& step);
        void DeleteStep(CucumberStep& step);
        void ClearStorage();

    private:
        infra::IntrusiveList<CucumberStep> stepList;
    };
}

#endif
