#ifndef SERVICES_CUCUMBER_STEP_MACRO_HPP 
#define SERVICES_CUCUMBER_STEP_MACRO_HPP

#include "services/cucumber/CucumberWireProtocolServer.hpp"
#include "services/cucumber/CucumberStepStorage.hpp"
#include "infra/util/BoundedVector.hpp"

#define CONCAT2(x, y) x ## y 
#define CONCAT(x, y) CONCAT2(x, y)

#define CLASSNAME CONCAT(Step_, __LINE__)
#define VARNAME CONCAT(step_, __LINE__)

#define GIVEN(NAME) DEFINESTEP(NAME)
#define WHEN(NAME) DEFINESTEP(NAME)
#define THEN(NAME) DEFINESTEP(NAME)

#define DEFINESTEP(NAME)                                                                                                                               \
namespace services                                                                                                                                     \
{                                                                                                                                                      \
    class CLASSNAME : public services::CucumberStep                                                                                                    \
    {                                                                                                                                                  \
    public:                                                                                                                                            \
        CLASSNAME()                                                                                                                                    \
            : services::CucumberStep(NAME)                                                                                                             \
        {                                                                                                                                              \
            services::CucumberStepStorage::Instance().AddStep(*this);                                                                                  \
        }                                                                                                                                              \
                                                                                                                                                       \
    public:                                                                                                                                            \
        void Invoke(infra::JsonArray& arguments, infra::Function<void(bool)> onDone)                                                                   \
        {                                                                                                                                              \
            invokeArguments = &arguments;                                                                                                              \
            Execute(onDone);                                                                                                                           \
        }                                                                                                                                              \
    private:                                                                                                                                           \
        void Execute(infra::Function<void(bool)> onDone);                                                                                              \
    };                                                                                                                                                 \
}                                                                                                                                                      \
namespace                                                                                                                                              \
{                                                                                                                                                      \
    static services::CLASSNAME VARNAME;                                                                                                                \
}                                                                                                                                                      \
void services::CLASSNAME::Execute(infra::Function<void(bool)> onDone)

#endif