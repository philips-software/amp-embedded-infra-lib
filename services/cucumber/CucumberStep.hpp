#ifndef SERVICES_CUCUMBER_STEP_HPP 
#define SERVICES_CUCUMBER_STEP_HPP

#include "infra/syntax/Json.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/syntax/JsonStreamingParser.hpp"
#include "services/cucumber/CucumberContext.hpp"

namespace services
{
    class CucumberStep
        : public infra::IntrusiveList<CucumberStep>::NodeType 
    {
    public:
        explicit CucumberStep(infra::BoundedConstString stepName);
        virtual ~CucumberStep() = default;
        CucumberStep& operator=(const CucumberStep& other) = delete;
        CucumberStep(CucumberStep& other) = delete;

        bool operator==(const CucumberStep& other) const;

        infra::BoundedConstString StepName() const;

        infra::Optional<infra::JsonString> GetTableArgument(infra::BoundedConstString fieldName);
        bool ContainsTableArgument(infra::BoundedConstString fieldName);
        infra::Optional<infra::JsonString> GetStringArgument(uint8_t argumentNumber);
        bool HasStringArguments();
        bool ContainsStringArgument(uint8_t index);
        uint16_t NrArguments();

        virtual void Invoke(infra::JsonArray& arguments) = 0;
        services::CucumberContext& Context();

    protected:
        infra::JsonArray* invokeArguments = nullptr;

    private:
        infra::BoundedConstString stepName;
    };

    class StringArgumentVisitor
        : public infra::JsonArrayVisitor
    {
    public:
        StringArgumentVisitor(size_t argNumber);

        virtual void VisitString(infra::BoundedConstString value) override;
        virtual JsonArrayVisitor* VisitArray(infra::JsonSubArrayParser& parser) override;

        infra::Optional<infra::JsonString>& MatchedArgument();
    private:
        bool tableArgument = false;
        size_t argNumber = 0;
        size_t argCount = 0;
        infra::BoundedString::WithStorage<48> matchBuffer;
        infra::Optional<infra::JsonString> matchedArgument = infra::none;
    };

    class TableArgumentVisitor
        : public infra::JsonArrayVisitor
    {
    public:
        TableArgumentVisitor(infra::BoundedConstString field);

        virtual void VisitString(infra::BoundedConstString value) override;
        virtual JsonArrayVisitor* VisitArray(infra::JsonSubArrayParser& parser) override;

        infra::Optional<infra::JsonString>& MatchedArgument();

    private:
        bool tableArgument = false;
        bool match = false;
        infra::BoundedConstString field;
        infra::Optional<infra::JsonString> matchedArgument = infra::none;
    };
}

#endif
