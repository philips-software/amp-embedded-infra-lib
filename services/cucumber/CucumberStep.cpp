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
            for (uint8_t stringArgumentCount = 0; stringArgumentCount < nrArgs; ++stringArgumentCount)
                ++iterator;
    }

    bool CucumberStep::ContainsTableArgument(infra::BoundedConstString fieldName) const
    {
        return GetTableArgument(fieldName) != infra::none;
    }

    infra::JsonArray CucumberStep::GetTable() const
    {
        infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
        SkipOverStringArguments(argumentIterator);
        return argumentIterator->Get<infra::JsonArray>();
    }

    infra::Optional<infra::JsonString> CucumberStep::GetTableArgument(infra::BoundedConstString fieldName) const
    {
        infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
        SkipOverStringArguments(argumentIterator);

        if (argumentIterator != invokeArguments->end())
            for (infra::JsonArrayIterator rowIterator = argumentIterator->Get<infra::JsonArray>().begin(); rowIterator != invokeArguments->end(); ++rowIterator)
                if (fieldName == rowIterator->Get<infra::JsonArray>().begin()->Get<infra::JsonString>())
                {
                    infra::JsonArrayIterator collumnIterator = rowIterator->Get<infra::JsonArray>().begin();
                    ++collumnIterator;
                    if (collumnIterator != rowIterator->Get<infra::JsonArray>().end())
                        return infra::MakeOptional(collumnIterator->Get<infra::JsonString>());
                }
        return infra::none;
    }

    bool CucumberStep::HasStringArguments() const
    {
        return StepName().find(R"('%s')") != infra::BoundedString::npos || StepName().find("%d") != infra::BoundedString::npos || StepName().find("%b") != infra::BoundedString::npos;
    }

    bool CucumberStep::ContainsStringArgument(uint8_t index) const
    {
        return GetStringArgument(index) != infra::none;
    }

    uint16_t CucumberStep::NrArguments() const
    {
        uint8_t nrArguments = 0;
        size_t strArgPos = StepName().find(R"('%s')", 0);
        size_t intArgPos = StepName().find("%d", 0);
        size_t boolArgPos = StepName().find("%b", 0);
        while (strArgPos != infra::BoundedString::npos)
        {
            ++nrArguments;
            strArgPos = StepName().find(R"('%s')", ++strArgPos);
        }
        while (intArgPos != infra::BoundedString::npos)
        {
            ++nrArguments;
            intArgPos = StepName().find("%d", ++intArgPos);
        }
        while (boolArgPos != infra::BoundedString::npos)
        {
            ++nrArguments;
            boolArgPos = StepName().find("%b", ++boolArgPos);
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
            ++nrFields;
            for (infra::JsonArrayIterator rowIterator = argumentIterator->Get<infra::JsonArray>().begin(); rowIterator != invokeArguments->end(); ++rowIterator)
                ++nrFields;
        }
        return nrFields;
    }

    infra::Optional<infra::JsonString> CucumberStep::GetStringArgument(uint8_t argumentNumber) const
    {
        if (invokeArguments->begin() != invokeArguments->end())
        {
            infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
            uint8_t argumentCount = 0;
            while (argumentIterator != invokeArguments->end() && argumentCount != argumentNumber)
            {
                ++argumentIterator;
                ++argumentCount;
            }
            if (argumentCount == argumentNumber)
                return infra::MakeOptional(argumentIterator->Get<infra::JsonString>());
        }
        return infra::none;
    }

    infra::Optional<uint32_t> CucumberStep::GetUIntegerArgument(uint8_t argumentNumber) const
    {
        auto optionalStringArgument = GetStringArgument(argumentNumber);

        if (optionalStringArgument)
        {
            infra::BoundedString::WithStorage<10> stringArgument;
            optionalStringArgument->ToString(stringArgument);
            infra::StringInputStream stream(stringArgument);
            uint32_t argument;
            stream >> argument;
            return infra::MakeOptional(argument);
        }

        return infra::none;
    }

    infra::Optional<bool> CucumberStep::GetBooleanArgument(uint8_t argumentNumber) const
    {
        auto optionalStringArgument = GetStringArgument(argumentNumber);

        if (optionalStringArgument)
        {
            infra::BoundedString::WithStorage<5> stringArgument;
            optionalStringArgument->ToString(stringArgument);
            return infra::MakeOptional(stringArgument == "true");
        }

        return infra::none;
    }
}
