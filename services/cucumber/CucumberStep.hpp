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
        virtual ~CucumberStep() = default;
        CucumberStep& operator=(const CucumberStep& other) = delete;
        CucumberStep(CucumberStep& other) = delete;

        bool operator==(const CucumberStep& other) const;

        infra::BoundedConstString StepName() const;
        infra::BoundedConstString SourceLocation() const;

        infra::JsonArray GetTable();
        infra::Optional<infra::JsonString> GetTableArgument(infra::BoundedConstString fieldName);
        bool ContainsTableArgument(infra::BoundedConstString fieldName);
        infra::Optional<infra::JsonString> GetStringArgument(uint8_t argumentNumber);
        infra::Optional<int32_t> GetIntegerArgument(uint8_t argumentNumber);
        bool HasStringArguments() const;
        bool ContainsStringArgument(uint8_t index);
        uint16_t NrArguments() const;
        uint16_t NrFields() const;

        virtual void Invoke(infra::JsonArray& arguments) = 0;
        services::CucumberContext& Context();

    protected:
        infra::JsonArray* invokeArguments = nullptr;

    private:
        void SkipOverStringArguments(infra::JsonArrayIterator& iterator) const;

        template <class T>
        infra::Optional<T> GetArgument(uint8_t argumentNumber);

        infra::BoundedConstString stepName;
        infra::BoundedConstString sourceLocation;
    };
}

#endif
