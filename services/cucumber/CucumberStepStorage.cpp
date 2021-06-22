#include "services/cucumber/CucumberStepStorage.hpp"

namespace services
{
    CucumberStepStorage& CucumberStepStorage::Instance()
    {
        static CucumberStepStorage instance;
        return instance;
    }

    void CucumberStepStorage::SkipStringArgument(infra::BoundedString::iterator& iterator, const infra::BoundedString& nameToMatch, int16_t& offsetCounter) const
    {
        while (iterator != nameToMatch.end() && *iterator != '\'')
        {
            ++iterator;
            ++offsetCounter;
        }
        offsetCounter -= 2;
    }

    void CucumberStepStorage::SkipIntegerArgument(infra::BoundedString::iterator& iterator, const infra::BoundedString& nameToMatch, int16_t& offsetCounter) const
    {
        while (iterator != nameToMatch.end() && *iterator >= '0' && *iterator <= '9')
        {
            ++iterator;
            ++offsetCounter;
        }
        offsetCounter -= 2;
    }

    bool CucumberStepStorage::MatchesStepName(CucumberStep& step, const infra::BoundedString& nameToMatch)
    {
        constexpr auto intMarker = "%d";
        constexpr auto stringMarker = R"('%s')";
        int16_t sizeOffset = 0;
        infra::BoundedConstString stepName = step.StepName();

        auto nameToMatchIterator = nameToMatch.begin();
        for (auto stepNameIterator = stepName.begin(); stepNameIterator != stepName.end(); ++stepNameIterator, ++nameToMatchIterator)
        {
            if (*stepNameIterator != *nameToMatchIterator)
            {
                if (infra::BoundedConstString(stepNameIterator - 1, std::strlen(stringMarker)) == stringMarker)
                {
                    SkipStringArgument(nameToMatchIterator, nameToMatch, sizeOffset);
                    stepNameIterator += std::strlen(stringMarker);
                    nameToMatchIterator += 2;
                }
                else if (infra::BoundedConstString(stepNameIterator, std::strlen(intMarker)) == intMarker)
                {
                    SkipIntegerArgument(nameToMatchIterator, nameToMatch, sizeOffset);
                    stepNameIterator += std::strlen(intMarker);
                }
                else
                    return false;
            }
        }

        return stepName.size() + sizeOffset == nameToMatch.size();
    }

    services::CucumberStepStorage::Match CucumberStepStorage::MatchStep(const infra::BoundedString& nameToMatch)
    {
        Match matchResult{};
        uint8_t nrStepMatches = 0;
        uint8_t count = 0;
 
        for (auto& step : stepList)
        {
            if (MatchesStepName(step, nameToMatch))
            {
                matchResult.id = count;
                nrStepMatches++;
            }
            count++;
        }
        if (nrStepMatches >= 2)
            matchResult.result = StepMatchResult::Duplicate;
        else if (nrStepMatches == 0)
            matchResult.result = StepMatchResult::Fail;
        else
        {
            matchResult.result = StepMatchResult::Success;
            matchResult.step = &GetStep(matchResult.id);
        }

        return matchResult;
    }

    CucumberStep& CucumberStepStorage::GetStep(uint32_t id)
    {
        really_assert(id <= stepList.size());
        auto step = stepList.begin();
        while (id-- > 0)
            ++step;
        return *step;
    }

    void CucumberStepStorage::AddStep(const CucumberStep& step)
    {
        stepList.push_back(step);
    }

    void CucumberStepStorage::DeleteStep(const CucumberStep& step)
    {
        stepList.erase(step);
    }

    void CucumberStepStorage::ClearStorage()
    {
        stepList.clear();
    }
}
