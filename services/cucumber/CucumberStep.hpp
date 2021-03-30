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
        CucumberStep(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName);

        ~CucumberStep() {}

        bool operator==(const CucumberStep& other) const;

        uint8_t Id();
        void SetId(uint8_t);
        infra::JsonArray& MatchArguments();
        void SetMatchArguments(infra::JsonArray arguments);
        infra::JsonArray& InvokeArguments();
        void SetInvokeArguments(infra::JsonArray arguments);
        infra::BoundedString& StepName();
        void StepName(infra::BoundedString stepName);
        infra::BoundedString& MatchArgumentsBuffer();
        uint8_t NrRows();
        uint8_t NrCollumns();


        static uint8_t NrSteps();
        static void SetNrSteps(uint8_t nrSteps);

        template<class T>
        T* GetStringArgument(uint8_t argumentNumber);
        bool ContainsStringArguments();
        uint8_t NrStringArguments();
        infra::JsonArray ParseArguments(const infra::BoundedString& nameToMatch, infra::BoundedString& arrayBuffer);

        template <class T>
        T* GetTableArgument(const infra::BoundedString& fieldName, infra::JsonArray& arguments);
        void GetTableDimensions();

        virtual bool Invoke(infra::JsonArray& arguments) = 0;

    private:
        uint8_t id;
        infra::JsonArray matchArguments;
        infra::JsonArray invokeArguments;
        infra::BoundedString::WithStorage<256> stepName;
        infra::BoundedString::WithStorage<256> matchArgumentsBuffer;

        uint8_t nrRows;
        uint8_t nrCollumns;

        static uint8_t nrSteps;
    };

    template<class T>
    inline T* CucumberStep::GetStringArgument(uint8_t argumentNumber)
    {
        T* result = nullptr;
        if (invokeArguments.begin() != invokeArguments.end())
        {
            infra::JsonArrayIterator argumentIterator(invokeArguments.begin());
            uint8_t argumentCount = 0;
            for (; argumentIterator != invokeArguments.end() && argumentCount != argumentNumber; argumentIterator++, argumentCount++);
            if (argumentCount == argumentNumber)
                result = &argumentIterator->Get<infra::JsonString>();
        }
        return result;
    }

    template<class T>
    inline T* CucumberStep::GetTableArgument(const infra::BoundedString& fieldName, infra::JsonArray& arguments)
    {
        T* result = nullptr;
        infra::JsonArrayIterator argumentIterator(arguments.begin());
        if (ContainsStringArguments())
            for (uint8_t stringArgumentCount = 0; stringArgumentCount < NrStringArguments(); stringArgumentCount++, argumentIterator++);
        if (argumentIterator != arguments.end())
            for (infra::JsonArrayIterator rowIterator = argumentIterator->Get<infra::JsonArray>().begin(); rowIterator != arguments.end(); rowIterator++)
                if (fieldName == rowIterator->Get<infra::JsonArray>().begin()->Get<infra::JsonString>())
                {
                    infra::JsonArrayIterator collumnIterator = rowIterator->Get<infra::JsonArray>().begin();
                    collumnIterator++;
                    result = &collumnIterator->Get<infra::JsonString>();
                }
        return result;
    }
    
}

#endif