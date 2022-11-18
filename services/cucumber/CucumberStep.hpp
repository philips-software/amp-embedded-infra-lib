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

        bool ContainsTableArgument(infra::BoundedConstString fieldName) const;
        infra::JsonArray GetTable() const;
        infra::Optional<infra::JsonString> GetTableArgument(infra::BoundedConstString fieldName) const;
        infra::Optional<infra::JsonString> GetStringArgument(uint8_t argumentNumber) const;
        infra::Optional<uint32_t> GetUIntegerArgument(uint8_t argumentNumber) const;
        infra::Optional<bool> GetBooleanArgument(uint8_t argumentNumber) const;
        bool HasStringArguments() const;
        bool ContainsStringArgument(uint8_t index) const;
        uint16_t NrArguments() const;
        uint16_t NrFields() const;

        virtual void Invoke(infra::JsonArray& arguments) = 0;
        services::CucumberContext& Context();

    protected:
        infra::JsonArray* invokeArguments = nullptr;

    private:
        void SkipOverStringArguments(infra::JsonArrayIterator& iterator) const;

        infra::BoundedConstString stepName;
        infra::BoundedConstString sourceLocation;
    };
}

#endif
