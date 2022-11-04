#ifndef HAL_RESET_HPP
#define HAL_RESET_HPP

namespace hal
{
    class Reset
    {
    protected:
        Reset() = default;
        Reset(const Reset& other) = delete;
        Reset& operator=(const Reset& other) = delete;
        ~Reset() = default;

    public:
        virtual void ResetModule(const char* resetReason) = 0;
    };
}

#endif
