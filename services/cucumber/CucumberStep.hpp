#ifndef SERVICES_CUCUMBER_STEP_HPP 
#define SERVICES_CUCUMBER_STEP_HPP

#include "infra/syntax/Json.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/tracer/TracingOutputStream.hpp"

namespace services
{
    class CucumberStep
        : public infra::IntrusiveList<CucumberStep>::NodeType 
    {
    public:
        CucumberStep(const services::CucumberStep& step);
        CucumberStep(const infra::BoundedString& stepName);

        ~CucumberStep() {}

        bool operator==(const CucumberStep& other) const;

        infra::JsonArray& MatchArguments();
        void SetMatchArguments(infra::JsonArray arguments);
        infra::BoundedString& StepName();
        void StepName(infra::BoundedString stepName);
        infra::BoundedString& MatchArgumentsBuffer();

        static uint8_t NrSteps();
        static void SetNrSteps(uint8_t nrSteps);

        infra::JsonString GetStringArgument(uint8_t argumentNumber);
        bool HasStringArguments();
        bool ContainsStringArgument(uint8_t index);
        uint8_t NrStringArguments();
        infra::JsonArray ParseMatchArguments(const infra::BoundedString& nameToMatch);

        infra::JsonString GetTableArgument(const infra::BoundedString& fieldName);
        bool ContainsTableArgument(const infra::BoundedString& fieldName);

        virtual bool Invoke(infra::JsonArray& arguments) = 0;

    protected:
        infra::JsonArray* invokeArguments;

    private:
        infra::JsonArray matchArguments;
        infra::BoundedString::WithStorage<256> matchArgumentsBuffer;
        infra::BoundedString::WithStorage<256> stepName;

        static uint8_t nrSteps;
    };
}

#endif