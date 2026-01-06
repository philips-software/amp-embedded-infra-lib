#include "infra/util/ReallyAssert.hpp"

// #ifdef EMIL_HOST_BUILD
namespace infra
{
    static AssertionFailureHandler customHandler = nullptr;

    void RegisterAssertionFailureHandler(AssertionFailureHandler handler)
    {
        customHandler = std::move(handler);
    }

    void HandleAssertionFailure(const char* condition, const char* file, int line)
    {
        if (customHandler)
            customHandler(condition, file, line);
    }
}

// #endif
