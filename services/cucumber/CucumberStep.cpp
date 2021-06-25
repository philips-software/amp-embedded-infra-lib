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

    bool CucumberStep::ContainsTableArgument(infra::BoundedConstString fieldName)
    {
        return GetTableArgument(fieldName) != infra::none;
    }

    infra::Optional<infra::JsonString> CucumberStep::GetTableArgument(infra::BoundedConstString fieldName)
    {
        TableArgumentVisitor visitor(fieldName);
        infra::JsonStreamingArrayParser::WithBuffers<1, 32, 4> parser(visitor);
        parser.Feed(invokeArguments->ObjectString());
        return visitor.MatchedArgument();
    }

    bool CucumberStep::HasStringArguments() const
    {
        return StepName().find("\'%s\'") != infra::BoundedString::npos || StepName().find("%d") != infra::BoundedString::npos;
    }

    bool CucumberStep::ContainsStringArgument(uint8_t index)
    {
        return GetStringArgument(index) != infra::none;
    }

    infra::Optional<infra::JsonString> CucumberStep::GetStringArgument(uint8_t argumentNumber)
    {
        StringArgumentVisitor visitor(argumentNumber);
        infra::JsonStreamingArrayParser::WithBuffers<1, 32, 4> arrayParser(visitor);
        arrayParser.Feed(invokeArguments->ObjectString());
        return visitor.MatchedArgument();
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

    StringArgumentVisitor::StringArgumentVisitor(size_t argNumber)
        : argNumber(argNumber)
    {}

    void StringArgumentVisitor::VisitString(infra::BoundedConstString value)
    {
        if (argCount == argNumber)
            matchedArgument.Emplace(infra::JsonString(value));

        if (!tableArgument)
            argCount++;
        value.clear();
    }

    infra::JsonArrayVisitor* StringArgumentVisitor::VisitArray(infra::JsonSubArrayParser& parser)
    {
        tableArgument = true;
        return this;
    }

    infra::Optional<infra::JsonString>& StringArgumentVisitor::MatchedArgument()
    {
        return matchedArgument;
    }

    TableArgumentVisitor::TableArgumentVisitor(infra::BoundedConstString field)
        : field(field)
    {}

    void TableArgumentVisitor::VisitString(infra::BoundedConstString value)
    {
        if (match)
        {
            matchedArgument.Emplace(infra::JsonString(value));
            match = false;
        }

        if (tableArgument && value == field)
            match = true;
        value.clear();
    }

    infra::JsonArrayVisitor* TableArgumentVisitor::VisitArray(infra::JsonSubArrayParser& parser)
    {
        tableArgument = true;
        match = false;
        return this;
    }

    infra::Optional<infra::JsonString>& TableArgumentVisitor::MatchedArgument()
    {
        return matchedArgument;
    }
}
