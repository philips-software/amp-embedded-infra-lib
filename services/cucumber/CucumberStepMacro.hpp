#ifndef SERVICES_CUCUMBER_STEP_MACRO_HPP 
#define SERVICES_CUCUMBER_STEP_MACRO_HPP

#include "services\cucumber\CucumberWireProtocolServer.hpp"
#include "services\cucumber\CucumberStepStorage.hpp"
#include "infra/util/BoundedVector.hpp"

#define CONCAT2(x, y) x ## y 
#define CONCAT(x, y) CONCAT2(x, y)

#define CLASSNAME CONCAT(Step_, __LINE__)
#define VARNAME CONCAT(step_, __LINE__)

#define GIVEN(NAME, FUNC) DEFINESTEP(NAME, FUNC)
#define WHEN(NAME, FUNC) DEFINESTEP(NAME, FUNC)
#define THEN(NAME, FUNC) DEFINESTEP(NAME, FUNC)


#define DEFINESTEP(NAME, FUNC)                                                                                                                     \
                                                                                                                                                   \
    class CLASSNAME : public services::CucumberStep                                                                                                \
    {                                                                                                                                              \
    public:                                                                                                                                        \
        CLASSNAME()                                                                                                                                \
            : services::CucumberStep(NAME)                                                                                                         \
        {                                                                                                                                          \
            services::CucumberStepStorage::Instance().AddStep(*this);                                                                              \
        }                                                                                                                                          \
                                                                                                                                                   \
    public:                                                                                                                                        \
        bool Invoke(infra::JsonArray& arguments)                                                                                                   \
        {                                                                                                                                          \
            SetInvokeArguments(arguments);                                                                                                         \
            FUNC                                                                                                                                   \
        }                                                                                                                                          \
    };                                                                                                                                             \
                                                                                                                                                   \
    static CLASSNAME VARNAME;                                                                                          


#endif