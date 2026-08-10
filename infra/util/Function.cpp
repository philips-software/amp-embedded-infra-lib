#include "infra/util/Function.hpp"
#include "infra/util/AbortOnExecuteFunction.hpp"

namespace infra
{
    namespace detail
    {
        const InvokerFunctions<void(), INFRA_DEFAULT_FUNCTION_EXTRA_SIZE>::VirtualMethodTable* GetAbortOnExecuteSentinelTable()
        {
            return InvokerFunctions<void(), INFRA_DEFAULT_FUNCTION_EXTRA_SIZE>::template StaticVirtualMethodTable<AbortOnExecuteFunction>();
        }
    }

    const infra::Function<void()> emptyFunction = []() {
    };

    Execute::Execute(Function<void()> f)
    {
        f();
    }

    ExecuteOnDestruction::ExecuteOnDestruction(Function<void()> f)
        : f(f)
    {}

    ExecuteOnDestruction::~ExecuteOnDestruction()
    {
        f();
    }
}
