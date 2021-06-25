#ifndef SERVICES_CUCUMBER_STEP_MACRO_HPP 
#define SERVICES_CUCUMBER_STEP_MACRO_HPP

#include "services/cucumber/CucumberStep.hpp"
#include "services/cucumber/CucumberStepStorage.hpp"

#define CONCAT_(x, y) x ## y
#define CONCAT(x, y) CONCAT_(x, y)

#define CLASSNAME CONCAT(Step_, __LINE__)
#define VARNAME CONCAT(step_, __LINE__)

#define GIVEN(NAME) DEFINESTEP(NAME)
#define WHEN(NAME) DEFINESTEP(NAME)
#define THEN(NAME) DEFINESTEP(NAME)

#define DEFINESTEP(NAME)                                                                                 \
namespace                                                                                                \
{                                                                                                        \
    class CLASSNAME                                                                                      \
        : public services::CucumberStep                                                                  \
    {                                                                                                    \
    public:                                                                                              \
        CLASSNAME()                                                                                      \
            : services::CucumberStep(NAME)                                                               \
        {                                                                                                \
            services::CucumberStepStorage::Instance().AddStep(*this);                                    \
        }                                                                                                \
                                                                                                         \
    public:                                                                                              \
        virtual void Invoke(infra::JsonArray& arguments) override                                        \
        {                                                                                                \
            invokeArguments = &arguments;                                                                \
            Execute();                                                                                   \
        }                                                                                                \
                                                                                                         \
    private:                                                                                             \
        void Execute();                                                                                  \
                                                                                                         \
        void Success()                                                                                   \
        {                                                                                                \
            assert(Context().Contains("InvokeSuccess"));                                                 \
            Context().Get<infra::Function<void()>>("InvokeSuccess")();                                   \
        }                                                                                                \
                                                                                                         \
        void Error(infra::BoundedConstString failReason)                                                 \
        {                                                                                                \
            assert(Context().Contains("InvokeError"));                                                   \
            Context().Get<infra::Function<void(infra::BoundedConstString&)>>("InvokeError")(failReason); \
        }                                                                                                \
    };                                                                                                   \
                                                                                                         \
    volatile static CLASSNAME VARNAME;                                                                   \
}                                                                                                        \
                                                                                                         \
void CLASSNAME::Execute()

#endif
