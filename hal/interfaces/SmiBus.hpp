#ifndef HAL_SMI_BUS_HPP
#define HAL_SMI_BUS_HPP

#include <cstdint>

namespace hal
{
    class SmiBus
    {
    protected:
        SmiBus() = default;
        SmiBus(const SmiBus&) = delete;
        SmiBus& operator=(const SmiBus&) = delete;
        SmiBus() = default;

    public:
        virtual uint16_t Read(uint8_t phyAddress, uint16_t reg) const = 0;
        virtual void Write(uint8_t phyAddress, uint16_t reg, uint16_t value) = 0;
    };
}

#endif
