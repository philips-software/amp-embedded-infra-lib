#include "services/cucumber/CucumberStepStorage.hpp"

namespace services
{
    StepStorage::StepStorage() : nrStepMatches(0) {}

    StepStorage::~StepStorage()
    {
        services::CucumberStep::SetNrSteps(0);
    }

    bool StepStorage::CompareStepName(CucumberStep& step, const infra::BoundedString& nameToMatch)
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

    CucumberStep* StepStorage::MatchStep(const infra::BoundedString& nameToMatch)
    {
        nrStepMatches = 0;
        CucumberStep *returnStep = nullptr;
        for (auto& step : stepList)
            if (CompareStepName(step, nameToMatch))
            {
                returnStep = &step;
                nrStepMatches++;
                if (step.ContainsArguments())
                    step.SetMatchArguments(step.ParseArguments(nameToMatch, step.MatchArgumentsBuffer()));
            }
        if (nrStepMatches >= 2 || nrStepMatches == 0)
            return nullptr;
        else
            return returnStep;
    }

    CucumberStep* StepStorage::MatchStep(uint8_t id)
    {
        for (auto& step : stepList)
            if (step.Id() == id)
            {
                return &step;
            }
        return nullptr;
    }

    void StepStorage::AddStep(const CucumberStep& step)
    {
        this->stepList.push_front(step);
    }
}