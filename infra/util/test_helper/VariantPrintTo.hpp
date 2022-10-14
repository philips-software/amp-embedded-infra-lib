#ifndef INFRA_VARIANT_PRINT_TO_HPP
#define INFRA_VARIANT_PRINT_TO_HPP

#include "gtest/gtest-printers.h"
#include "infra/util/Variant.hpp"

namespace infra
{
    using testing::internal::PrintTo;

    struct PrintToVisitor
    {
        using ResultType = void;

        PrintToVisitor(std::ostream* os)
            : os(os)
        {}

        template<class T>
        void operator()(const T& value)
        {
            PrintTo(value, os);
        }

    private:
        std::ostream* os;
    };

    template<class... T>
    void PrintTo(const Variant<T...>& variant, std::ostream* os)
    {
        *os << "Variant[";
        PrintToVisitor visitor(os);
        ApplyVisitor(visitor, variant);
        *os << ']';
    }
}

#endif
