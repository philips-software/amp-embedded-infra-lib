#include "services/cucumber/CucumberStepStorage.hpp"

namespace
{
    template<class Iterator>
    std::size_t SkipStringArgument(Iterator& iterator, Iterator end)
    {
        std::size_t skippedSize = 0;

        while (iterator != end && *iterator != '\'')
        {
            ++iterator;
            ++skippedSize;
        }

        return skippedSize;
    }

    template<class Iterator>
    std::size_t SkipIntegerArgument(Iterator& iterator, Iterator end)
    {
        std::size_t skippedSize = 0;

        while (iterator != end && *iterator >= '0' && *iterator <= '9')
        {
            ++iterator;
            ++skippedSize;
        }
        
        return skippedSize;
    }

    template<class Iterator>
    std::size_t SkipBooleanArgument(Iterator& iterator, Iterator end)
    {
        std::size_t skippedSize = 0;

        if (*iterator == 't')
        {
            if (*++iterator == 'r' && *++iterator == 'u' && *++iterator == 'e')
                skippedSize = 4;
        }
        else if (*iterator == 'f')
        {
            if (*++iterator == 'a' && *++iterator == 'l' && *++iterator == 's' && *++iterator == 'e')
                skippedSize = 5;
        }

        ++iterator;

        return skippedSize;
    }

    template<class Iterator>
    std::size_t SkipMarker(Iterator& iterator, Iterator end)
    {
        std::size_t skippedSize = 0;

        while (iterator != end && (*iterator == '%' || *iterator == 's' || *iterator == 'd' || *iterator == 'b'))
        {
            ++iterator;
            ++skippedSize;
        }

        return skippedSize;
    }
}

namespace services
{
    CucumberStepStorage& CucumberStepStorage::Instance()
    {
        static CucumberStepStorage instance;
        return instance;
    }

    bool CucumberStepStorage::MatchesStepName(CucumberStep& step, const infra::BoundedString& nameToMatch)
    {
        std::size_t sizeOffset = 0;
        auto stepName = step.StepName();
        auto nameToMatchIterator = nameToMatch.begin();

        for (auto stepNameIterator = stepName.begin(); stepNameIterator != stepName.end();)
        {
            if (*stepNameIterator != *nameToMatchIterator)
            {
                constexpr auto intMarker = "%d";
                constexpr auto boolMarker = "%b";
                constexpr auto stringMarker = R"('%s')";

                if (infra::BoundedConstString(stepNameIterator - 1, std::strlen(stringMarker)) == stringMarker)
                {
                    sizeOffset += SkipStringArgument(nameToMatchIterator, nameToMatch.end());
                    sizeOffset -= SkipMarker(stepNameIterator, stepName.end());
                }
                else if (infra::BoundedConstString(stepNameIterator, std::strlen(intMarker)) == intMarker)
                {
                    sizeOffset += SkipIntegerArgument(nameToMatchIterator, nameToMatch.end());
                    sizeOffset -= SkipMarker(stepNameIterator, stepName.end());
                }
                else if (infra::BoundedConstString(stepNameIterator, std::strlen(boolMarker)) == boolMarker)
                {
                    sizeOffset += SkipBooleanArgument(nameToMatchIterator, nameToMatch.end());
                    sizeOffset -= SkipMarker(stepNameIterator, stepName.end());
                }
                else
                    return false;
            }
            else
                ++stepNameIterator, ++nameToMatchIterator;
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
        really_assert(id < stepList.size());
        return *std::next(stepList.begin(), id);
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
