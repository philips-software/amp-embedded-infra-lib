#include "hal/interfaces/Can.hpp"

namespace hal
{
    bool Can::Id::Is11BitId() const
    {
        return (id & indicator29Bit) == 0;
    }

    bool Can::Id::Is29BitId() const
    {
        return !Is11BitId();
    }

    uint32_t Can::Id::Get11BitId() const
    {
        assert(Is11BitId());

        return id;
    }

    uint32_t Can::Id::Get29BitId() const
    {
        assert(Is29BitId());

        return id ^ indicator29Bit;
    }

    bool Can::Id::operator==(const Id& other) const
    {
        return id == other.id;
    }

    bool Can::Id::operator!=(const Id& other) const
    {
        return !(*this == other);
    }
}
