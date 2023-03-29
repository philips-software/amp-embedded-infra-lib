#ifndef HAL_I2C_REGISTER_ACCESS_HPP
#define HAL_I2C_REGISTER_ACCESS_HPP

#include "hal/interfaces/I2c.hpp"
#include "infra/util/WithStorage.hpp"

namespace hal
{
    template<class T>
    class I2cMasterRegisterAccess;

    using I2cMasterRegisterAccessByte = I2cMasterRegisterAccess<uint8_t>;
    using I2cMasterRegisterAccessHalfWord = I2cMasterRegisterAccess<uint16_t>;
    using I2cMasterRegisterAccessWord = I2cMasterRegisterAccess<uint32_t>;

    template<class T>
    class I2cMasterRegisterAccess
    {
    public:
        I2cMasterRegisterAccess(I2cMaster& i2cMaster, I2cAddress address);

        void ReadRegister(T dataRegister, infra::ByteRange data, const infra::Function<void()>& onDone);
        void WriteRegister(T dataRegister, infra::ConstByteRange data, const infra::Function<void()>& onDone);

    private:
        I2cMaster& i2cMaster;
        I2cAddress address;

        T dataRegister = 0;
        infra::ByteRange readData;
        infra::ConstByteRange writeData;
        infra::Function<void()> onDone;
    };
}

#endif
