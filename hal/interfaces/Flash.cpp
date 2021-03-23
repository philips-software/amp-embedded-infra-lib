#include "hal/interfaces/Flash.hpp"

namespace hal
{
    template class FlashBase<uint32_t>;
    template class FlashBase<uint64_t>;
}
