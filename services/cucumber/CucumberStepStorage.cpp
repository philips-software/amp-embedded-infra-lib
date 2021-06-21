#include "services/cucumber/CucumberStepStorage.hpp"

namespace services
{
    CucumberStepStorage& CucumberStepStorage::Instance()
    {
        static CucumberStepStorage instance;
        return instance;
    }

    void CucumberStepStorage::IterateThroughStringArgument(infra::BoundedString::iterator& iterator, const infra::BoundedString& nameToMatch, int16_t& offsetCounter)
    {
        while (iterator != nameToMatch.end() && *iterator != '\'')
        {
            iterator++;
            offsetCounter++;
        }
        offsetCounter -= 2;
    }

    void CucumberStepStorage::IterateThroughIntegerArgument(infra::BoundedString::iterator& iterator, const infra::BoundedString& nameToMatch, int16_t& offsetCounter)
    {
        while (iterator != nameToMatch.end() && *(iterator + 1) >= '0' && *(iterator + 1) <= '9')
        {
            iterator++;
            offsetCounter++;
        }
        offsetCounter--;
    }

    bool CucumberStepStorage::MatchesStepName(CucumberStep& step, const infra::BoundedString& nameToMatch)
    {
        uint16_t count = 0; 
        int16_t sizeOffset = 0;
        for (infra::BoundedString::iterator c = nameToMatch.begin(); c != nameToMatch.end(); c++, count++)
        {
            if (step.StepName()[count] != *c)
            {
                if ((count - 1) >= 0 && step.StepName().find("\'%s\'", (count - 1)) == (count - 1))
                {
                    count += 2;
                    IterateThroughStringArgument(c, nameToMatch, sizeOffset);
                }
                else if (step.StepName().find("%d", count) == count)
                {
                    count++;
                    IterateThroughIntegerArgument(c, nameToMatch, sizeOffset);
                }
                else
                    return false;
            }
        }
        return step.StepName().size() + sizeOffset == nameToMatch.size();
    }

    services::CucumberStepStorage::Match CucumberStepStorage::MatchStep(const infra::BoundedString& nameToMatch)
    {
        Match matchResult;
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
            matchResult.result = Duplicate;
        else if (nrStepMatches == 0)
            matchResult.result = Fail;
        else
        {
            matchResult.result = Success;
            matchResult.step = &GetStep(matchResult.id);
        }
        return matchResult;
    }

    CucumberStep& CucumberStepStorage::GetStep(uint8_t id)
    {
        really_assert(id <= stepList.size());
        auto step = stepList.begin();
        while (id-- > 0)
            ++step;
        return *step;
    }

    void CucumberStepStorage::AddStep(CucumberStep& step)
    {
        this->stepList.push_back(step);
    }

    void CucumberStepStorage::DeleteStep(CucumberStep& step)
    {
        this->stepList.erase(step);
    }

    void CucumberStepStorage::ClearStorage()
    {
        this->stepList.clear();
    }
}
