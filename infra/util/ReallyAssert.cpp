#include "ReallyAssert.hpp"

#ifdef EMIL_HOST_BUILD
namespace infra
{
    static AssertionFailureHandler customHandler = nullptr;

    void RegisterAssertionFailureHandler(AssertionFailureHandler handler)
    {
        customHandler = handler;
    }

    [[noreturn]] void HandleAssertionFailure(const char* condition, const char* file, int line)
    {
        if (customHandler)
            customHandler(condition, file, line);

        std::abort();
    }
}
#endif
