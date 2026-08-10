#include "infra/util/Function.hpp"
#include "infra/util/LogAndAbort.hpp"

namespace infra::detail
{
    namespace
    {
        struct AbortOnExecuteFunction
        {
            [[noreturn]] void operator()() const
            {
                LOG_AND_ABORT("Aborting on uninitialized function call");
            }
        };
    }

    const InvokerFunctions<void(), INFRA_DEFAULT_FUNCTION_EXTRA_SIZE>::VirtualMethodTable* GetAbortOnExecuteSentinelTable()
    {
        return InvokerFunctions<void(), INFRA_DEFAULT_FUNCTION_EXTRA_SIZE>::template StaticVirtualMethodTable<AbortOnExecuteFunction>();
    }
}
