#include "services/cucumber/CucumberStep.hpp"

namespace services
{
    uint8_t CucumberStep::nrSteps = 0;

    uint8_t CucumberStep::Id()
    {
        return id;
    }

    void CucumberStep::SetId(uint8_t idIn)
    {
        id = idIn;
    }

    infra::JsonArray& CucumberStep::MatchArguments()
    {
        return matchArguments;
    }

    void CucumberStep::SetMatchArguments(infra::JsonArray arguments)
    {
        matchArguments = arguments;
    }

    infra::JsonArray& CucumberStep::InvokeArguments()
    {
        return invokeArguments;
    }

    void CucumberStep::SetInvokeArguments(infra::JsonArray arguments)
    {
        invokeArguments = arguments;
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

    uint8_t CucumberStep::NrRows()
    {
        return nrRows;
    }

    uint8_t CucumberStep::NrCollumns()
    {
        return nrCollumns;
    }

    uint8_t CucumberStep::NrSteps()
    {
        return nrSteps;
    }

    void CucumberStep::SetNrSteps(uint8_t nrOfSteps)
    {
        nrSteps = nrOfSteps;
    }

    bool CucumberStep::ContainsStringArguments()
    {
        if (StepName().find("\'%s\'") != infra::BoundedString::npos || StepName().find("%d") != infra::BoundedString::npos)
            return true;
        return false;
    }

    uint8_t CucumberStep::NrStringArguments()
    {
        uint8_t nrArguments = 0;
        {
            for (size_t argPos = 0; StepName().find("\'%s\'", argPos) != infra::BoundedString::npos || StepName().find("%d", argPos) != infra::BoundedString::npos; argPos++, nrArguments++)
            {
                if (StepName().find("\'%s\'", argPos) != infra::BoundedString::npos)
                {
                    argPos = StepName().find("\'%s\'", argPos);
                }
                else if (StepName().find("%d", argPos) != infra::BoundedString::npos)
                {
                    argPos = StepName().find("%d", argPos);
                }
            }
        }
        return nrArguments;
    }

    infra::JsonArray CucumberStep::ParseArguments(const infra::BoundedString& nameToMatch, infra::BoundedString& arrayBuffer)
    {
        {
            arrayBuffer.clear();
            infra::JsonArrayFormatter::WithStringStream arguments(infra::inPlace, arrayBuffer);
            for (uint32_t argPos = 0, argOffset = 0; StepName().find("\'%s\'", argPos) != infra::BoundedString::npos || StepName().find("%d", argPos) != infra::BoundedString::npos; argPos++)
            {
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
        }
        return infra::JsonArray(arrayBuffer);
    }

    void CucumberStep::GetTableDimensions()
    {
        if (invokeArguments.begin() != invokeArguments.end())
        {
            infra::JsonArrayIterator argumentIterator(invokeArguments.begin());
            if (ContainsStringArguments())
            {
                for (uint8_t stringArgumentCount = 0; stringArgumentCount < NrStringArguments(); stringArgumentCount++, argumentIterator++);
            }
            if (argumentIterator != invokeArguments.end())
            {
                infra::JsonArrayIterator rowIterator = argumentIterator->Get<infra::JsonArray>().begin();
                infra::JsonArray collumnArray = rowIterator->Get<infra::JsonArray>();
                for (auto string : JsonStringArray(collumnArray))
                    nrCollumns++;
                for (; rowIterator != invokeArguments.end(); nrRows++, rowIterator++);
            }
            else
            {
                nrRows = 0;
                nrCollumns = 0;
            }
        }
        else
        {
            nrRows = 0;
            nrCollumns = 0;
        }
    }

    bool CucumberStep::operator==(const CucumberStep& other) const
    {
        if (stepName == other.stepName)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    CucumberStep::CucumberStep(const services::CucumberStep& step)
        : matchArguments(step.matchArguments)
        , stepName(step.stepName)
    {}

    CucumberStep::CucumberStep(const infra::BoundedString& stepName)
        : matchArguments(infra::JsonArray("[]"))
        , stepName(stepName)
        , nrRows(0)
        , nrCollumns(0)
    {
        nrSteps++;
    }

    CucumberStep::CucumberStep(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
        : matchArguments(matchArguments)
        , stepName(stepName)
    {
        nrSteps++;
    }
}
