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

        template<class T>
        T* GetStringArgument(uint8_t argumentNumber, infra::JsonArray& arguments);
        bool ContainsStringArguments();
        uint8_t NrStringArguments();
        infra::JsonArray ParseMatchArguments(const infra::BoundedString& nameToMatch);

        template <class T>
        T* GetTableArgument(const infra::BoundedString& fieldName, infra::JsonArray& arguments);

        virtual bool Invoke(infra::JsonArray& arguments) = 0;

    private:
        infra::JsonArray matchArguments;
        infra::BoundedString::WithStorage<256> matchArgumentsBuffer;
        infra::BoundedString::WithStorage<256> stepName;

        static uint8_t nrSteps;
    };

    template<class T>
    inline T* CucumberStep::GetStringArgument(uint8_t argumentNumber, infra::JsonArray& arguments)
    {
        T* result = nullptr;
        if (arguments.begin() != arguments.end())
        {
            infra::JsonArrayIterator argumentIterator(arguments.begin());
            uint8_t argumentCount = 1;
            for (; argumentIterator != arguments.end() && argumentCount != argumentNumber; argumentIterator++, argumentCount++);
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
                    if (collumnIterator != rowIterator->Get<infra::JsonArray>().end())
                        result = &collumnIterator->Get<infra::JsonString>();
                }
        return result;
    }
    
}

#endif