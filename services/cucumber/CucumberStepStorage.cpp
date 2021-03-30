#include "services/cucumber/CucumberStepStorage.hpp"

namespace services
{
    CucumberStepStorage::CucumberStepStorage()
    {}

    CucumberStepStorage::~CucumberStepStorage()
    {
        services::CucumberStep::SetNrSteps(0);
        ClearStorage();
    }

    services::CucumberStepStorage::StepMatchResult& CucumberStepStorage::MatchResult()
    {
        return matchResult;
    }

    void CucumberStepStorage::SetMatchResult(StepMatchResult result)
    {
        matchResult = result;
    }

    uint8_t& CucumberStepStorage::MatchId()
    {
        return matchId;
    }

    void CucumberStepStorage::SetMatchId(uint8_t& id)
    {
        matchId = id;
    }

    bool CucumberStepStorage::CompareStepName(CucumberStep& step, const infra::BoundedString& nameToMatch)
   {
        uint8_t count = 0;
        uint8_t sizeOffset = 0;
        for (infra::BoundedString::iterator c = nameToMatch.begin(); c != nameToMatch.end(); ++c, count++)
        {
            if (step.StepName()[count] != *c)
            {
                if (((count - 1) >= 0 && (count + 2) <= step.StepName().size()) && (step.StepName()[count - 1] == '\'' && step.StepName()[count] == '%' && step.StepName()[count + 1] == 's' && step.StepName()[count + 2] == '\'' && *(c - 1) == '\''))
                {
                    count += 2;
                    for (; c != nameToMatch.end() && *c != '\''; c++, sizeOffset++);
                    sizeOffset -= 2;
                }
                else if ((count + 1 <= step.StepName().size()) && (step.StepName()[count] == '%' && step.StepName()[count + 1] == 'd'))
                {
                    count++;
                    for (; c != nameToMatch.end() && *(c + 1) >= '0' && *(c + 1) <= '9'; c++, sizeOffset++);
                    sizeOffset--;
                }
                else
                    return false;
            }
        }
        if (step.StepName().size() + sizeOffset == nameToMatch.size())
            return true;
        else
            return false;
    }

    void CucumberStepStorage::MatchStep(const infra::BoundedString& nameToMatch)
    {
        uint8_t nrStepMatches = 0, count = 0;
        for (auto& step : stepList)
        {
            if (CompareStepName(step, nameToMatch))
            {
                SetMatchId(count);
                nrStepMatches++;
                if (step.ContainsStringArguments() && nrStepMatches < 2)
                    step.SetMatchArguments(step.ParseArguments(nameToMatch, step.MatchArgumentsBuffer()));
            }
            count++;
        }
        if (nrStepMatches >= 2)
            SetMatchResult(services::CucumberStepStorage::duplicate);
        else if ((nrStepMatches == 0 && count == stepList.size()))
            SetMatchResult(services::CucumberStepStorage::fail);
        else
            SetMatchResult(services::CucumberStepStorage::success);
    }

    CucumberStep& CucumberStepStorage::GetStep(uint8_t id)
    {
        really_assert(id <= stepList.size());
        auto& step = stepList.begin();
        while (id-- > 0)
            ++step;
        return *step;
    }

    uint8_t CucumberStepStorage::GetId(CucumberStep& step)
    {
        uint8_t count = 0;
        for (auto& stepIterator : stepList)
        {
            if (&stepIterator == &step)
                return count;
            count++;
        }
        return count;
    }

    void CucumberStepStorage::AddStep(CucumberStep& step)
    {
        this->stepList.push_back(step);
        step.SetId(stepList.size() - 1);
    }

    void CucumberStepStorage::DeleteStep(CucumberStep& step)
    {
        stepList.erase(step);
    }

    void CucumberStepStorage::ClearStorage()
    {
        stepList.clear();
    }
}