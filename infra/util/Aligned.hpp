#ifndef INFRA_ALIGNED_HPP
#define INFRA_ALIGNED_HPP

#include "infra/util/MemoryRange.hpp"
#include <array>

namespace infra
{
    template<class As, class Type>
    class Aligned
    {
    public:
        Aligned(Type value)
        {
            Copy(ReinterpretCastMemoryRange<As>(MakeRange(&value, &value + 1)), MakeRange(this->value));
        }

        operator Type() const
        {
            Type result;
            Copy(MakeRange(value), ReinterpretCastMemoryRange<As>(MakeRange(&value, &value + 1)));
            return result;
        }

    private:
        std::array<As, sizeof(Type) / sizeof(As)> value;
    };
}

#endif
