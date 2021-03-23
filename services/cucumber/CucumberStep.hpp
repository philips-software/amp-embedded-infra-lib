#ifndef SERVICES_CUCUMBER_STEP_HPP 
#define SERVICES_CUCUMBER_STEP_HPP

#include "infra/syntax/Json.hpp"
#include "infra/syntax/JsonFormatter.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/IntrusiveForwardList.hpp"

namespace services
{
    class CucumberStep
        : public infra::IntrusiveList<CucumberStep>::NodeType
    {
    public:
        CucumberStep(const infra::BoundedString& stepName);
        CucumberStep(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName);

        ~CucumberStep() {}

        uint8_t Id();
        void SetId(uint8_t);
        infra::JsonArray& MatchArguments();
        void SetMatchArguments(infra::JsonArray arguments);
        infra::JsonArray& TableHeaders();
        void SetTableHeaders(infra::JsonArray arguments);
        infra::BoundedString& StepName();
        void StepName(infra::BoundedString stepName);
        infra::BoundedString& MatchArgumentsBuffer();
        void Invoke(infra::JsonArray& arguments);

        bool ContainsArguments();
        uint8_t NrArguments();
        infra::JsonArray ParseArguments(const infra::BoundedString& nameToMatch, infra::BoundedString& arrayBuffer);

    private:
        uint8_t id;
        infra::JsonArray matchArguments;
        infra::JsonArray tableHeaders;
        infra::BoundedString::WithStorage<256> stepName;
        infra::BoundedString::WithStorage<256> matchArgumentsBuffer;

        static uint8_t nrSteps;
    };
}

#endif