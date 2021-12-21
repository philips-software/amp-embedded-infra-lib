#include "services/cucumber/CucumberStep.hpp"
#include "infra/stream/StringInputStream.hpp"

namespace services
{
    CucumberStep::CucumberStep(infra::BoundedConstString stepName, infra::BoundedConstString sourceLocation)
        : stepName(stepName)
        , sourceLocation(sourceLocation)
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

    infra::BoundedConstString CucumberStep::SourceLocation() const
    {
        return sourceLocation;
    }

    void CucumberStep::SkipOverStringArguments(infra::JsonArrayIterator& iterator) const
    {
        uint32_t nrArgs = NrArguments();
        if (HasStringArguments())
            for (uint8_t stringArgumentCount = 0; stringArgumentCount < nrArgs; stringArgumentCount++)
                iterator++;
    }

    bool CucumberStep::ContainsTableArgument(infra::BoundedConstString fieldName)
    {
        return GetTableArgument(fieldName) != infra::none;
    }

    infra::JsonArray CucumberStep::GetTable()
    {
        infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
        SkipOverStringArguments(argumentIterator);
        return argumentIterator->Get<infra::JsonArray>();
    }

    infra::Optional<infra::JsonString> CucumberStep::GetTableArgument(infra::BoundedConstString fieldName)
    {
        infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
        SkipOverStringArguments(argumentIterator);

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
        return StepName().find(R"('%s')") != infra::BoundedString::npos || StepName().find("%d") != infra::BoundedString::npos;
    }

    bool CucumberStep::ContainsStringArgument(uint8_t index)
    {
        return GetStringArgument(index) != infra::none;
    }

    uint16_t CucumberStep::NrArguments() const
    {
        uint8_t nrArguments = 0;
        size_t strArgPos = StepName().find(R"('%s')", 0);
        size_t intArgPos = StepName().find("%d", 0);
        while (strArgPos != infra::BoundedString::npos)
        {
            nrArguments++;
            strArgPos = StepName().find(R"('%s')", ++strArgPos);
        }
        while (intArgPos != infra::BoundedString::npos)
        {
            nrArguments++;
            intArgPos = StepName().find("%d", ++intArgPos);
        }
        return nrArguments;
    }

    uint16_t CucumberStep::NrFields() const
    {
        uint16_t nrFields = 0;
        infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
        SkipOverStringArguments(argumentIterator);

        if (argumentIterator != invokeArguments->end())
        {
            nrFields++;
            for (infra::JsonArrayIterator rowIterator = argumentIterator->Get<infra::JsonArray>().begin(); rowIterator != invokeArguments->end(); rowIterator++)
                nrFields++;
        }
        return nrFields;
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

    infra::Optional<uint32_t> CucumberStep::GetUIntegerArgument(uint8_t argumentNumber)
    {
        auto optionalStringArgument = GetStringArgument(argumentNumber);

        if (optionalStringArgument)
        {
            infra::BoundedString::WithStorage<10> stringArgument;
            optionalStringArgument->ToString(stringArgument);
            infra::StringInputStream stream(stringArgument);
            uint32_t argument;
            stream >> argument;
            return infra::Optional<uint32_t>(infra::inPlace, uint32_t(argument));
        }

        return infra::Optional<uint32_t>(infra::none);
    }
}
