#include "infra/util/AbortOnExecuteFunction.hpp"
#include "infra/util/LogAndAbort.hpp"

void infra::detail::AbortOnExecuteFunction::operator()() const
{
    LOG_AND_ABORT("Aborting on uninitialized function call");
}
