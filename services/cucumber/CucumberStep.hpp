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
        explicit CucumberStep(infra::BoundedConstString stepName);
        virtual ~CucumberStep() = default;
        CucumberStep& operator=(const CucumberStep& other) = delete;
        CucumberStep(CucumberStep& other) = delete;

        bool operator==(const CucumberStep& other) const;

        infra::BoundedConstString StepName() const;

        infra::Optional<infra::JsonString> GetTableArgument(const infra::BoundedString& fieldName);
        bool ContainsTableArgument(const infra::BoundedString& fieldName);
        infra::Optional<infra::JsonString> GetStringArgument(uint8_t argumentNumber);
        bool HasStringArguments();
        bool ContainsStringArgument(uint8_t index);
        uint16_t NrArguments();

        virtual void Invoke(infra::JsonArray& arguments) = 0;
        static services::CucumberContext& Context();

    protected:
        infra::JsonArray* invokeArguments = nullptr;

    private:
        void IterateThroughStringArguments(infra::JsonArrayIterator& iterator);

        infra::BoundedConstString stepName;
    };
}

#endif
