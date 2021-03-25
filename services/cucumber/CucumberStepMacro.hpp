#ifndef SERVICES_CUCUMBER_STEP_MACRO_HPP 
#define SERVICES_CUCUMBER_STEP_MACRO_HPP

#include "services\cucumber\CucumberWireProtocolServer.hpp"
#include "services\cucumber\CucumberStepStorage.hpp"
#include "infra/util/BoundedVector.hpp"

#define TOKENIZE(x, y) x ## y 

#define CLASSNAME TOKENIZE(Step_, __LINE__)
#define VARNAME TOKENIZE(step_, __LINE__)

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
            services::CucumberStepStorage::Instance().AddStep(*this);                                                                               \
        }                                                                                                                                          \
                                                                                                                                                   \
    public:                                                                                                                                        \
        void Invoke(infra::JsonArray& arguments)                                                                                                   \
        {                                                                                                                                          \
            FUNC                                                                                                                                   \
        }                                                                                                                                          \
    };                                                                                                                                             \
                                                                                                                                                   \
    static CLASSNAME VARNAME;                                                                                          


#endif