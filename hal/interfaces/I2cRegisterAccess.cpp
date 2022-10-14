#include "hal/interfaces/I2cRegisterAccess.hpp"

namespace hal
{
    I2cMasterRegisterAccess::I2cMasterRegisterAccess(I2cMaster& i2cMaster, I2cAddress address)
        : i2cMaster(i2cMaster)
        , address(address)
    {}

    void I2cMasterRegisterAccess::ReadRegister(uint8_t dataRegister, infra::ByteRange data, const infra::Function<void()>& onDone)
    {
        this->dataRegister = dataRegister;
        this->readData = data;
        this->onDone = onDone;

        i2cMaster.SendData(address, infra::MakeByteRange(this->dataRegister), hal::Action::repeatedStart, [this](hal::Result, uint32_t numberOfBytesSent)
            { i2cMaster.ReceiveData(address, readData, hal::Action::stop, [this](hal::Result)
                  { this->onDone(); }); });
    }

    void I2cMasterRegisterAccess::WriteRegister(uint8_t dataRegister, infra::ConstByteRange data, const infra::Function<void()>& onDone)
    {
        this->dataRegister = dataRegister;
        this->writeData = data;
        this->onDone = onDone;

        i2cMaster.SendData(address, infra::MakeByteRange(this->dataRegister), hal::Action::continueSession, [this](hal::Result, uint32_t numberOfBytesSent)
            { i2cMaster.SendData(address, writeData, hal::Action::stop, [this](hal::Result, uint32_t numberOfBytesSent)
                  { this->onDone(); }); });
    }
}
