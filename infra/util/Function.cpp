#include "infra/util/Function.hpp"

namespace infra::detail
{
    struct AbortOnExecuteFunction
    {
        [[noreturn]] void operator()() const;
    };
}

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
