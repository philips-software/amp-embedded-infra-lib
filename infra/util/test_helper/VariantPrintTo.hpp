#ifndef INFRA_VARIANT_PRINT_TO_HPP
#define INFRA_VARIANT_PRINT_TO_HPP

#include "gtest/gtest-printers.h"
#include <ostream>
#include <variant>

namespace infra
{
    using testing::internal::PrintTo;

    template<class... T>
    void PrintTo(const std::variant<T...>& variant, std::ostream* os)
    {
        *os << "Variant[";

        std::visit([os](const auto& value)
            {
                PrintTo(value, os);
            },
            variant);

        *os << ']';
    }
}

#endif
