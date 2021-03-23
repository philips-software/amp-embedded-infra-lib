#include "services/cucumber/CucumberStep.hpp"

namespace services
{
    uint8_t Step::nrSteps = 0;

    uint8_t Step::Id()
    {
        return id;
    }

    void Step::SetId(uint8_t idIn)
    {
        id = idIn;
    }

    infra::JsonArray& Step::MatchArguments()
    {
        return matchArguments;
    }

    void Step::SetMatchArguments(infra::JsonArray arguments)
    {
        matchArguments = arguments;
    }

    infra::JsonArray& Step::TableHeaders()
    {
        return this->tableHeaders;
    }

    void Step::SetTableHeaders(infra::JsonArray arguments)
    {
        tableHeaders = arguments;
    }

    infra::BoundedString& Step::StepName()
    {
        return stepName;
    }

    void Step::StepName(infra::BoundedString stepName)
    {
        stepName = stepName;
    }

    infra::BoundedString& Step::MatchArgumentsBuffer()
    {
        return matchArgumentsBuffer;
    }

    bool Step::ContainsArguments()
    {
        if (StepName().find("\'%s\'") != infra::BoundedString::npos || StepName().find("%d") != infra::BoundedString::npos)
            return true;
        return false;
    }

    uint8_t Step::NrArguments()
    {
        uint8_t nrArguments = 0;
        {
            uint8_t argPos = 0;
            do
            {
                if (StepName().find("\'%s\'", argPos) != infra::BoundedString::npos)
                {
                    argPos = StepName().find("\'%s\'", argPos);
                    argPos++;
                    nrArguments++;
                }
                if (StepName().find("%d", argPos) != infra::BoundedString::npos)
                {
                    argPos = StepName().find("%d", argPos);
                    argPos++;
                    nrArguments++;
                }
            } while (StepName().find("\'%s\'", argPos) != infra::BoundedString::npos || StepName().find("%d", argPos) != infra::BoundedString::npos);
        }
        return nrArguments;
    }

    infra::JsonArray Step::ParseArguments(const infra::BoundedString& nameToMatch, infra::BoundedString& arrayBuffer)
    {
        {
            arrayBuffer.clear();
            infra::JsonArrayFormatter::WithStringStream arguments(infra::inPlace, arrayBuffer);
            uint32_t argPos = 0;
            uint32_t argOffset = 0;
            do
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
                        argPos++;
                        argOffset += stringStream.Storage().size() - 2;
                    }
                }
                if (StepName().find("%d", argPos) != infra::BoundedString::npos)
                {
                    argPos = StepName().find("%d", argPos);
                    infra::JsonObjectFormatter subObject(arguments.SubObject());
                    infra::StringOutputStream::WithStorage<32> digitStream;
                    for (uint8_t i = 0; (nameToMatch[argPos + argOffset + i] >= '0' && nameToMatch[argPos + argOffset + i] <= '9'); i++)
                        digitStream << nameToMatch[argPos + argOffset + i];
                    subObject.Add("val", digitStream.Storage());
                    subObject.Add("pos", argPos + argOffset);
                    argPos++;
                    argOffset += digitStream.Storage().size() - 2;
                }
            } while (StepName().find("\'%s\'", argPos) != infra::BoundedString::npos || StepName().find("%d", argPos) != infra::BoundedString::npos);
        }
        infra::JsonArray argumentArray(arrayBuffer);
        return argumentArray;
    }

    void Step::Invoke(infra::JsonArray& arguments)
    {
        
    }

    Step::Step(const infra::BoundedString& stepName)
        : id(nrSteps)
        , matchArguments(infra::JsonArray("[]"))
        , tableHeaders(infra::JsonArray("[]"))
        , stepName(stepName)
    {
        nrSteps++;
    }

    Step::Step(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName)
        : id(nrSteps)
        , matchArguments(matchArguments)
        , tableHeaders(tableHeaders)
        , stepName(stepName)
    {
        nrSteps++;
    }
}
