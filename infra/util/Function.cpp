#include "infra/util/Function.hpp"

namespace infra
{
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
