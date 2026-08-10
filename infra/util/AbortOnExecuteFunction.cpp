#include "infra/util/LogAndAbort.hpp"

namespace infra::detail
{
    struct AbortOnExecuteFunction
    {
        [[noreturn]] void operator()() const;
    };
}

[[noreturn]] void infra::detail::AbortOnExecuteFunction::operator()() const
{
    LOG_AND_ABORT("Aborting on uninitialized function call");
}
