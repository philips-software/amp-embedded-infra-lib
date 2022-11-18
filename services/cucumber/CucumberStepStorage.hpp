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
            found,
            notFound,
            duplicate
        };

        struct StepMatch
        {
            StepMatchResult result;
            uint32_t id;
            const CucumberStep* step;
        };

        static CucumberStepStorage& Instance();

        services::CucumberStepStorage::StepMatch MatchStep(infra::BoundedConstString stepText);

        const CucumberStep& GetStep(uint32_t id);
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
