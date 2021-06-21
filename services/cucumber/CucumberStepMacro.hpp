#ifndef SERVICES_CUCUMBER_STEP_MACRO_HPP 
#define SERVICES_CUCUMBER_STEP_MACRO_HPP

#include "services/cucumber/CucumberWireProtocolServer.hpp"

#define CONCAT2(x, y) x ## y
#define CONCAT(x, y) CONCAT2(x, y)

#define CLASSNAME CONCAT(Step_, __LINE__)
#define VARNAME CONCAT(step_, __LINE__)

#define GIVEN(NAME, TYPENAME) DEFINESTEP(NAME, TYPENAME)
#define WHEN(NAME, TYPENAME) DEFINESTEP(NAME, TYPENAME)
#define THEN(NAME, TYPENAME) DEFINESTEP(NAME, TYPENAME)

#define DEFINESTEP(NAME, TYPENAME)                                                                                                                 \
namespace {                                                                                                                                        \
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
    virtual void Invoke(infra::JsonArray& arguments) override                                                                                      \
    {                                                                                                                                              \
        invokeArguments = &arguments;                                                                                                              \
        Execute();                                                                                                                                 \
    }                                                                                                                                              \
                                                                                                                                                   \
    TYPENAME& StepContext()                                                                                                                        \
    {                                                                                                                                              \
        return stepContext;                                                                                                                        \
    }                                                                                                                                              \
                                                                                                                                                   \
private:                                                                                                                                           \
    void Execute();                                                                                                                                \
                                                                                                                                                   \
    TYPENAME stepContext;                                                                                                                          \
                                                                                                                                                   \
    void Success()                                                                                                                                 \
    {                                                                                                                                              \
        assert(services::CucumberContext::Instance().Contains("InvokeSuccess"));                                                                   \
        this->Context().Get<infra::Function<void()>>("InvokeSuccess").Invoke(std::make_tuple());                                                   \
    }                                                                                                                                              \
                                                                                                                                                   \
    void Error(infra::BoundedConstString failReason)                                                                                               \
    {                                                                                                                                              \
        assert(services::CucumberContext::Instance().Contains("InvokeError"));                                                                     \
        this->Context().Get<infra::Function<void(infra::BoundedConstString&)>>("InvokeError").Invoke(failReason);                                  \
    }                                                                                                                                              \
                                                                                                                                                   \
};                                                                                                                                                 \
}                                                                                                                                                  \
namespace                                                                                                                                          \
{                                                                                                                                                  \
     volatile static CLASSNAME VARNAME;                                                                                                            \
}                                                                                                                                                  \
                                                                                                                                                   \
void CLASSNAME::Execute()

#endif
