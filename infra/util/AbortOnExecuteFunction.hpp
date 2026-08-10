#ifndef INFRA_ABORT_ON_EXECUTE_FUNCTION_HPP
#define INFRA_ABORT_ON_EXECUTE_FUNCTION_HPP


namespace infra
{
    namespace detail
    {
        struct AbortOnExecuteFunction
        {
            void operator()() const;
        };   
    }
}

#endif
