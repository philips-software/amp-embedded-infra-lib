#include "hal/interfaces/Can.hpp"

namespace hal
{
    Can::Id::Id(uint32_t id)
        : id(id)
    {}

    Can::Id Can::Id::Create11BitId(uint32_t id)
    {
        return Can::Id(id);
    }

    Can::Id Can::Id::Create29BitId(uint32_t id)
    {
        return Can::Id(id | indicator29Bit);
    }

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
}
