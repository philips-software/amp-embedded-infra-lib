#ifndef SERVICES_CUCUMBER_STEP_HPP
#define SERVICES_CUCUMBER_STEP_HPP

#include "infra/syntax/Json.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "services/cucumber/CucumberContext.hpp"

namespace services
{
    class CucumberStep
        : public infra::IntrusiveList<CucumberStep>::NodeType
    {
    public:
        CucumberStep(infra::BoundedConstString stepName, infra::BoundedConstString sourceLocation);
        CucumberStep& operator=(const CucumberStep& other) = delete;
        CucumberStep(CucumberStep& other) = delete;
        virtual ~CucumberStep() = default;

        infra::BoundedConstString StepName() const;
        infra::BoundedConstString SourceLocation() const;

        bool Matches(infra::BoundedConstString name) const;

        bool ContainsTableArgument(infra::JsonArray& arguments, infra::BoundedConstString fieldName) const;
        infra::JsonArray GetTable(infra::JsonArray& arguments) const;
        infra::Optional<infra::JsonString> GetTableArgument(infra::JsonArray& arguments, infra::BoundedConstString fieldName) const;
        infra::Optional<infra::JsonString> GetStringArgument(infra::JsonArray& arguments, uint8_t argumentNumber) const;
        infra::Optional<uint32_t> GetUIntegerArgument(infra::JsonArray& arguments, uint8_t argumentNumber) const;
        infra::Optional<bool> GetBooleanArgument(infra::JsonArray& arguments, uint8_t argumentNumber) const;
        bool HasArguments() const;
        bool ContainsArgument(infra::JsonArray& arguments, uint8_t index) const;
        uint16_t NrArguments() const;
        uint16_t NrFields(infra::JsonArray& arguments) const;

        virtual void Invoke(infra::JsonArray& arguments) const = 0;
        services::CucumberContext& Context() const;

    private:
        void SkipOverArguments(infra::JsonArrayIterator& iterator) const;

    private:
        infra::BoundedConstString stepName;
        infra::BoundedConstString sourceLocation;
    };
}

#endif
