#include "services/cucumber/CucumberStep.hpp"

namespace services
{
    CucumberStep::CucumberStep(infra::BoundedConstString stepName)
        : stepName(stepName)
    {}

    bool CucumberStep::operator==(const CucumberStep& other) const
    {
        return stepName == other.stepName;
    }

    services::CucumberContext& CucumberStep::Context()
    {
        return services::CucumberContext::Instance();
    }

    infra::BoundedConstString CucumberStep::StepName() const
    {
        return stepName;
    }

    void CucumberStep::IterateThroughStringArguments(infra::JsonArrayIterator& iterator)
    {
        uint32_t nrArgs = NrArguments();
        if (HasStringArguments())
            for (uint8_t stringArgumentCount = 0; stringArgumentCount < nrArgs; stringArgumentCount++)
                iterator++;
    }

    bool CucumberStep::ContainsTableArgument(const infra::BoundedString& fieldName)
    {
        return GetTableArgument(fieldName) != infra::none;
    }

    infra::Optional<infra::JsonString> CucumberStep::GetTableArgument(const infra::BoundedString& fieldName)
    {
        infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
        IterateThroughStringArguments(argumentIterator);

        if (argumentIterator != invokeArguments->end())
            for (infra::JsonArrayIterator rowIterator = argumentIterator->Get<infra::JsonArray>().begin(); rowIterator != invokeArguments->end(); rowIterator++)
                if (fieldName == rowIterator->Get<infra::JsonArray>().begin()->Get<infra::JsonString>())
                {
                    infra::JsonArrayIterator collumnIterator = rowIterator->Get<infra::JsonArray>().begin();
                    collumnIterator++;
                    if (collumnIterator != rowIterator->Get<infra::JsonArray>().end())
                        return infra::Optional<infra::JsonString>(infra::inPlace, collumnIterator->Get<infra::JsonString>());
                }
        return infra::Optional<infra::JsonString>(infra::none);
    }

    bool CucumberStep::HasStringArguments() const
    {
        return StepName().find("\'%s\'") != infra::BoundedString::npos || StepName().find("%d") != infra::BoundedString::npos;
    }

    bool CucumberStep::ContainsStringArgument(uint8_t index)
    {
        return GetStringArgument(index) != infra::none;
    }

    uint16_t CucumberStep::NrArguments() const
    {
        uint8_t nrArguments = 0;
        size_t strArgPos = StepName().find("\'%s\'", 0);
        size_t intArgPos = StepName().find("%d", 0);
        while (strArgPos != infra::BoundedString::npos)
        {
            nrArguments++;
            strArgPos = StepName().find("\'%s\'", ++strArgPos);
        }
        while (intArgPos != infra::BoundedString::npos)
        {
            nrArguments++;
            intArgPos = StepName().find("%d", ++intArgPos);
        }
        return nrArguments;
    }

    infra::Optional<infra::JsonString> CucumberStep::GetStringArgument(uint8_t argumentNumber)
    {
        if (invokeArguments->begin() != invokeArguments->end())
        {
            infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
            uint8_t argumentCount = 0;
            while (argumentIterator != invokeArguments->end() && argumentCount != argumentNumber)
            {
                argumentIterator++;
                argumentCount++;
            }
            if (argumentCount == argumentNumber)
                return infra::Optional<infra::JsonString>(infra::inPlace, argumentIterator->Get<infra::JsonString>());
        }
        return infra::Optional<infra::JsonString>(infra::none);
    }
}
