#ifndef SERVICES_CUCUMBER_STEP_STORAGE_HPP
#define SERVICES_CUCUMBER_STEP_STORAGE_HPP

#include "infra/util/BoundedString.hpp"
#include "services/cucumber/CucumberStep.hpp"

namespace services
{
    class CucumberStepStorage
    {
    public:
        enum class StepMatchResult : uint8_t
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

        services::CucumberStepStorage::Match MatchStep(infra::BoundedConstString stepText);

        CucumberStep& GetStep(uint32_t id);
        void AddStep(CucumberStep& step);

    protected:
        CucumberStepStorage() = default;
        CucumberStepStorage& operator=(const CucumberStepStorage& other) = delete;
        CucumberStepStorage(CucumberStepStorage& other) = delete;
        virtual ~CucumberStepStorage() = default;

    private:
        infra::IntrusiveList<CucumberStep> steps;
    };
}

#endif
