#ifndef INFRA_ABORT_ON_EXECUTE_FUNCTION_HPP
#define INFRA_ABORT_ON_EXECUTE_FUNCTION_HPP

namespace infra::detail
{
    struct AbortOnExecuteFunction
    {
        [[noreturn]] void operator()() const;
    };
}

#endif
