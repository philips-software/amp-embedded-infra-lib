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
        CucumberStep(const infra::BoundedString& stepName);
        CucumberStep(const infra::JsonArray& matchArguments, const infra::JsonArray& tableHeaders, const infra::BoundedString& stepName);

        ~CucumberStep() {}

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
        T* GetTableArgument(const infra::BoundedString& collumnHeader, uint8_t rowNumber);
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
    inline T* CucumberStep::GetTableArgument(const infra::BoundedString& collumnHeader, uint8_t rowNumber)
    {
        uint8_t collumnMatch = 0;
        uint8_t nrMatches = 0;
        T* result = nullptr;

        infra::JsonArrayIterator argumentIterator(invokeArguments.begin());
        if (ContainsStringArguments())
        {
            for (uint8_t stringArgumentCount = 0; stringArgumentCount < NrStringArguments(); stringArgumentCount++, argumentIterator++);
        }
        if (argumentIterator != invokeArguments.end())
        {
            infra::JsonArrayIterator rowIterator = argumentIterator->Get<infra::JsonArray>().begin();
            infra::JsonArray collumnArray = rowIterator->Get<infra::JsonArray>();
            uint8_t collumnCount = 1;
            for (auto string : JsonStringArray(collumnArray))
            {
                if (string == collumnHeader)
                    collumnMatch = collumnCount;
                collumnCount++;
            }
            if (collumnMatch != 0)
                for (uint8_t rowCount = 0; rowIterator != invokeArguments.end(); rowCount++, rowIterator++)
                {
                    if (rowCount == rowNumber)
                    {
                        collumnCount = 1;
                        infra::JsonArray collumnArray = rowIterator->Get<infra::JsonArray>();
                        infra::JsonArrayIterator value(collumnArray.begin());
                        for (; value != collumnArray.end() && collumnCount != collumnMatch; value++, collumnCount++);
                        if (collumnCount == collumnMatch)
                            result = &value->Get<infra::JsonString>();
                    }
                }
        }
        return result;
    }
    
}

#endif