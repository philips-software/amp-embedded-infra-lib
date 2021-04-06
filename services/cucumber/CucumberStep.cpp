#include "services/cucumber/CucumberStep.hpp"

namespace services
{
    uint8_t CucumberStep::nrSteps = 0;

    infra::JsonArray& CucumberStep::MatchArguments()
    {
        return matchArguments;
    }

    void CucumberStep::SetMatchArguments(infra::JsonArray arguments)
    {
        matchArguments = arguments;
    }

    infra::BoundedString& CucumberStep::StepName()
    {
        return stepName;
    }

    void CucumberStep::StepName(infra::BoundedString stepName)
    {
        stepName = stepName;
    }

    infra::BoundedString& CucumberStep::MatchArgumentsBuffer()
    {
        return matchArgumentsBuffer;
    }

    uint8_t CucumberStep::NrSteps()
    {
        return nrSteps;
    }

    void CucumberStep::SetNrSteps(uint8_t nrOfSteps)
    {
        nrSteps = nrOfSteps;
    }

    bool CucumberStep::ContainsTableArgument(const infra::BoundedString& fieldName)
    {
        infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
        if (HasStringArguments())
            for (uint8_t stringArgumentCount = 0; stringArgumentCount < NrStringArguments(); stringArgumentCount++, argumentIterator++);
        if (argumentIterator != invokeArguments->end())
            for (infra::JsonArrayIterator rowIterator = argumentIterator->Get<infra::JsonArray>().begin(); rowIterator != invokeArguments->end(); rowIterator++)
                if (fieldName == rowIterator->Get<infra::JsonArray>().begin()->Get<infra::JsonString>())
                {
                    infra::JsonArrayIterator collumnIterator = rowIterator->Get<infra::JsonArray>().begin();
                    collumnIterator++;
                    if (collumnIterator != rowIterator->Get<infra::JsonArray>().end())
                        return true;
                }
        return false;
    }

    infra::JsonString CucumberStep::GetTableArgument(const infra::BoundedString& fieldName)
    {
        infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
        if (HasStringArguments())
            for (uint8_t stringArgumentCount = 0; stringArgumentCount < NrStringArguments(); stringArgumentCount++, argumentIterator++);
        if (argumentIterator != invokeArguments->end())
            for (infra::JsonArrayIterator rowIterator = argumentIterator->Get<infra::JsonArray>().begin(); rowIterator != invokeArguments->end(); rowIterator++)
                if (fieldName == rowIterator->Get<infra::JsonArray>().begin()->Get<infra::JsonString>())
                {
                    infra::JsonArrayIterator collumnIterator = rowIterator->Get<infra::JsonArray>().begin();
                    collumnIterator++;
                    if (collumnIterator != rowIterator->Get<infra::JsonArray>().end())
                        return collumnIterator->Get<infra::JsonString>();
                }
        return infra::JsonString("");
    }

    bool CucumberStep::HasStringArguments()
    {
        if (StepName().find("\'%s\'") != infra::BoundedString::npos || StepName().find("%d") != infra::BoundedString::npos)
            return true;
        return false;
    }

    bool CucumberStep::ContainsStringArgument(uint8_t index)
    {
        if (invokeArguments->begin() != invokeArguments->end())
        {
            infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
            uint8_t argumentCount = 0;
            for (; argumentIterator != invokeArguments->end() && argumentCount != index; argumentIterator++, argumentCount++);
            if (argumentCount == index)
                return true;
        }
        return false;
    }

    uint8_t CucumberStep::NrStringArguments()
    {
        uint8_t nrArguments = 0;
        for (size_t argPos = 0; StepName().find("\'%s\'", argPos) != infra::BoundedString::npos || StepName().find("%d", argPos) != infra::BoundedString::npos; argPos++, nrArguments++)
            if (StepName().find("\'%s\'", argPos) != infra::BoundedString::npos)
                argPos = StepName().find("\'%s\'", argPos);
            else if (StepName().find("%d", argPos) != infra::BoundedString::npos)
                argPos = StepName().find("%d", argPos);
        return nrArguments;
    }

    infra::JsonString CucumberStep::GetStringArgument(uint8_t argumentNumber)
    {
        if (invokeArguments->begin() != invokeArguments->end())
        {
            infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
            uint8_t argumentCount = 0;
            for (; argumentIterator != invokeArguments->end() && argumentCount != argumentNumber; argumentIterator++, argumentCount++);
            if (argumentCount == argumentNumber)
                return argumentIterator->Get<infra::JsonString>();
        }
        return infra::JsonString("");
    }

    infra::JsonArray CucumberStep::ParseMatchArguments(const infra::BoundedString& nameToMatch)
    {
        {
            matchArgumentsBuffer.clear();
            infra::JsonArrayFormatter::WithStringStream arguments(infra::inPlace, matchArgumentsBuffer);
            for (uint32_t argPos = 0, argOffset = 0; StepName().find("\'%s\'", argPos) != infra::BoundedString::npos || StepName().find("%d", argPos) != infra::BoundedString::npos; argPos++)
                if (StepName().find("\'%s\'", argPos) != infra::BoundedString::npos)
                {
                    argPos = StepName().find("\'%s\'", argPos);
                    if (nameToMatch[argPos + argOffset] == '\'')
                    {
                        infra::JsonObjectFormatter subObject(arguments.SubObject());
                        infra::StringOutputStream::WithStorage<32> stringStream;
                        for (uint8_t i = 1; nameToMatch[argPos + argOffset + i] != '\''; i++)
                            stringStream << nameToMatch[argPos + argOffset + i];
                        subObject.Add("val", stringStream.Storage());
                        subObject.Add("pos", argPos + argOffset + 1);
                        argOffset += stringStream.Storage().size() - 2;
                    }
                }
                else if (StepName().find("%d", argPos) != infra::BoundedString::npos)
                {
                    argPos = StepName().find("%d", argPos);
                    infra::JsonObjectFormatter subObject(arguments.SubObject());
                    infra::StringOutputStream::WithStorage<32> digitStream;
                    for (uint8_t i = 0; (nameToMatch[argPos + argOffset + i] >= '0' && nameToMatch[argPos + argOffset + i] <= '9'); i++)
                        digitStream << nameToMatch[argPos + argOffset + i];
                    subObject.Add("val", digitStream.Storage());
                    subObject.Add("pos", argPos + argOffset);
                    argOffset += digitStream.Storage().size() - 2;
                }
        }
        return infra::JsonArray(matchArgumentsBuffer);
    }

    bool CucumberStep::operator==(const CucumberStep& other) const
    {
        if (stepName == other.stepName)
            return true;
        else
            return false;
    }

    CucumberStep::CucumberStep(const services::CucumberStep& step)
        : matchArguments(step.matchArguments)
        , stepName(step.stepName)
    {}

    CucumberStep::CucumberStep(const infra::BoundedString& stepName)
        : matchArguments(infra::JsonArray("[]"))
        , stepName(stepName)
    {
        nrSteps++;
    }
}
