#include "services/cucumber/CucumberStep.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/util/AutoResetFunction.hpp"

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

    void CucumberStepArguments::Invoke(infra::JsonArray& arguments)
    {
        invokeArguments = &arguments;
        Execute();
    }

    void CucumberStepArguments::SkipOverStringArguments(infra::JsonArrayIterator& iterator) const
    {
        uint32_t nrArgs = NrArguments();
        if (HasStringArguments())
            for (uint8_t stringArgumentCount = 0; stringArgumentCount < nrArgs; ++stringArgumentCount)
                ++iterator;
    }

    bool CucumberStepArguments::ContainsTableArgument(infra::BoundedConstString fieldName) const
    {
        return GetTableArgument(fieldName) != std::nullopt;
    }

    infra::JsonArray CucumberStepArguments::GetTable() const
    {
        infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
        SkipOverStringArguments(argumentIterator);
        return std::get<infra::JsonArray>(*argumentIterator);
    }

    std::optional<infra::JsonString> CucumberStepArguments::GetTableArgument(infra::BoundedConstString fieldName) const
    {
        infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
        SkipOverStringArguments(argumentIterator);

        if (argumentIterator != invokeArguments->end())
            for (infra::JsonArrayIterator rowIterator = std::get<infra::JsonArray>(*argumentIterator).begin(); rowIterator != invokeArguments->end(); ++rowIterator)
                if (fieldName == std::get<infra::JsonString>(*std::get<infra::JsonArray>(*rowIterator).begin()))
                {
                    infra::JsonArrayIterator collumnIterator = std::get<infra::JsonArray>(*rowIterator).begin();
                    ++collumnIterator;
                    if (collumnIterator != std::get<infra::JsonArray>(*rowIterator).end())
                        return std::make_optional(std::get<infra::JsonString>(*collumnIterator));
                }
        return std::nullopt;
    }

    bool CucumberStepArguments::HasStringArguments() const
    {
        return StepName().find(R"('%s')") != infra::BoundedString::npos || StepName().find("%d") != infra::BoundedString::npos || StepName().find("%b") != infra::BoundedString::npos;
    }

    bool CucumberStepArguments::ContainsStringArgument(uint8_t index) const
    {
        return GetStringArgument(index) != std::nullopt;
    }

    uint16_t CucumberStepArguments::NrArguments() const
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

    uint16_t CucumberStepArguments::NrFields() const
    {
        uint16_t nrFields = 0;
        infra::JsonArrayIterator argumentIterator(invokeArguments->begin());
        SkipOverStringArguments(argumentIterator);

        if (argumentIterator != invokeArguments->end())
        {
            ++nrFields;
            for (infra::JsonArrayIterator rowIterator = std::get<infra::JsonArray>(*argumentIterator).begin(); rowIterator != invokeArguments->end(); ++rowIterator)
                ++nrFields;
        }
        return nrFields;
    }

    std::optional<infra::JsonString> CucumberStepArguments::GetStringArgument(uint8_t argumentNumber) const
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
                return std::make_optional(std::get<infra::JsonString>(*argumentIterator));
        }
        return std::nullopt;
    }

    std::optional<uint32_t> CucumberStepArguments::GetUIntegerArgument(uint8_t argumentNumber) const
    {
        auto optionalStringArgument = GetStringArgument(argumentNumber);

        if (optionalStringArgument)
        {
            infra::BoundedString::WithStorage<10> stringArgument;
            optionalStringArgument->ToString(stringArgument);
            infra::StringInputStream stream(stringArgument);
            uint32_t argument;
            stream >> argument;
            return std::make_optional(argument);
        }

        return std::nullopt;
    }

    std::optional<bool> CucumberStepArguments::GetBooleanArgument(uint8_t argumentNumber) const
    {
        auto optionalStringArgument = GetStringArgument(argumentNumber);

        if (optionalStringArgument)
        {
            infra::BoundedString::WithStorage<5> stringArgument;
            optionalStringArgument->ToString(stringArgument);
            return std::make_optional(stringArgument == "true");
        }

        return std::nullopt;
    }

    void CucumberStepProgress::Success()
    {
        if (!std::exchange(isActive, false))
            std::abort();

        assert(Context().onSuccess);
        Context().onSuccess();
    }

    void CucumberStepProgress::Error(infra::BoundedConstString failReason)
    {
        if (!std::exchange(isActive, false))
            std::abort();

        assert(Context().onFailure);
        Context().onFailure(failReason);
    }

    void CucumberStepProgress::Invoke(infra::JsonArray& arguments)
    {
        isActive = true;

        CucumberStepArguments::Invoke(arguments);
    }
}
