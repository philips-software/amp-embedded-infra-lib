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

    infra::JsonArray& CucumberStep::TableHeaders()
    {
        return this->tableHeaders;
    }

    void CucumberStep::SetTableHeaders(infra::JsonArray arguments)
    {
        tableHeaders = arguments;
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

    bool CucumberStep::ContainsArguments()
    {
        if (StepName().find("\'%s\'") != infra::BoundedString::npos || StepName().find("%d") != infra::BoundedString::npos)
            return true;
        return false;
    }

    uint8_t CucumberStep::NrArguments()
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

    CucumberStep::CucumberStep(const infra::BoundedString& stepName)
        : id(nrSteps)
        , matchArguments(infra::JsonArray("[]"))
        , tableHeaders(infra::JsonArray("[]"))
        , stepName(stepName)
    {
        nrSteps++;
    }

    CucumberStep::CucumberStep(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
        : id(nrSteps)
        , matchArguments(matchArguments)
        , tableHeaders(tableHeaders)
        , stepName(stepName)
    {
        nrSteps++;
    }
}
