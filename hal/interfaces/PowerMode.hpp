#ifndef HAL_POWER_MODE_HPP
#define HAL_POWER_MODE_HPP

namespace hal
{
    class PowerMode
    {
    protected:
        PowerMode() = default;
        PowerMode(const PowerMode& other) = delete;
        PowerMode& operator=(const PowerMode& other) = delete;
        ~PowerMode() = default;

    public:
        virtual bool EnterStandby() = 0;
    };
}

#endif
