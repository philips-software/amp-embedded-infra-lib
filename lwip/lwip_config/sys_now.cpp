#include <cstdint>

extern "C" uint32_t HAL_GetTick();

extern "C" std::uint32_t sys_now()
{
    return HAL_GetTick();
}
