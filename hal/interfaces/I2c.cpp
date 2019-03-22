#include "hal/interfaces/I2c.hpp"

namespace hal
{
    I2cAddress::I2cAddress(uint16_t address)
        : address(address)
    {}

    bool I2cAddress::operator==(const I2cAddress& other) const
    {
        return other.address == this->address;
    }

    bool I2cAddress::operator!=(const I2cAddress& other) const
    {
        return !(*this == other);
    }
}
