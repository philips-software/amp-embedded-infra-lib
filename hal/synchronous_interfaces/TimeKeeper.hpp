#ifndef SYNCHRONOUS_HAL_TIME_KEEPER_HPP
#define SYNCHRONOUS_HAL_TIME_KEEPER_HPP

namespace hal
{
    class TimeKeeper
    {
    public:
        virtual bool Timeout() = 0;
        virtual void Reset() = 0;
    };
}

#endif
