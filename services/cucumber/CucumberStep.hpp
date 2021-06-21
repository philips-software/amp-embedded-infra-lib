#ifndef SERVICES_CUCUMBER_STEP_HPP 
#define SERVICES_CUCUMBER_STEP_HPP

#include "infra/syntax/JsonFormatter.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "services/cucumber/CucumberContext.hpp"

namespace services
{
    class CucumberStep
        : public infra::IntrusiveList<CucumberStep>::NodeType 
    {
    public:
        explicit CucumberStep(const infra::BoundedString& stepName);
        CucumberStep& operator=(const CucumberStep& other) = delete;
        CucumberStep(CucumberStep& other) = delete;
        virtual ~CucumberStep() = default;
        bool operator==(const CucumberStep& other) const;

        infra::BoundedString& StepName();

        infra::Optional<infra::JsonString> GetTableArgument(const infra::BoundedString& fieldName);
        bool ContainsTableArgument(const infra::BoundedString& fieldName);
        infra::Optional<infra::JsonString> GetStringArgument(uint8_t argumentNumber);
        bool HasStringArguments();
        bool ContainsStringArgument(uint8_t index);
        uint16_t NrArguments();

        virtual void Invoke(infra::JsonArray& arguments) = 0;
        static services::CucumberContext& Context();

    protected:
        infra::JsonArray* invokeArguments;
    private:
        void IterateThroughStringArguments(infra::JsonArrayIterator& iterator);

        infra::BoundedString::WithStorage<256> stepName;
    };
}

#endif