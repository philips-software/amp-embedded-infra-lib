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

        enum StepMatchResult
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

        CucumberStep& GetStep(uint8_t id);
        services::CucumberStepStorage::Match MatchStep(const infra::BoundedString& nameToMatch);
        void AddStep(CucumberStep& step);
        void DeleteStep(CucumberStep& step);
        void ClearStorage();
        bool MatchesStepName(CucumberStep& step, const infra::BoundedString& stepName);

    private:
        void IterateThroughStringArgument(infra::BoundedString::iterator& iterator, const infra::BoundedString& nameToMatch, int16_t& offsetCounter);
        void IterateThroughIntegerArgument(infra::BoundedString::iterator& iterator, const infra::BoundedString& nameToMatch, int16_t& offsetCounter);

        infra::IntrusiveList<CucumberStep> stepList;
    };
}

#endif