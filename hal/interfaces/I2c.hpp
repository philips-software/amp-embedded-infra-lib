#ifndef HAL_I2C_HPP
#define HAL_I2C_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"

namespace hal
{
    enum class Result
    {
        complete,
        partialComplete,
        busError
    };

    enum class Action
    {
        continueSession,
        repeatedStart,
        stop
    };

    enum class DataDirection
    {
        send,
        receive
    };

    class I2cAddress
    {
    public:
        explicit constexpr I2cAddress(uint16_t address)
            : address(address)
        {}

        bool operator==(const I2cAddress& other) const;
        bool operator!=(const I2cAddress& other) const;

        uint16_t address;
    };

    class I2cErrorPolicy
    {
    protected:
        I2cErrorPolicy() = default;
        I2cErrorPolicy(const I2cErrorPolicy& other) = delete;
        I2cErrorPolicy& operator=(const I2cErrorPolicy& other) = delete;
        ~I2cErrorPolicy() = default;

    public:
        virtual void DeviceNotFound() = 0;
        virtual void BusError() = 0;
        virtual void ArbitrationLost() = 0;
    };

    class I2cMaster
    {
    public:
        I2cMaster() = default;
        I2cMaster(const I2cMaster& other) = delete;
        I2cMaster& operator=(const I2cMaster& other) = delete;

    protected:
        ~I2cMaster() = default;

    public:
        virtual void SendData(I2cAddress address, infra::ConstByteRange data, Action nextAction, infra::Function<void(Result, uint32_t numberOfBytesSent)> onSent) = 0;
        virtual void ReceiveData(I2cAddress address, infra::ByteRange data, Action nextAction, infra::Function<void(Result)> onReceived) = 0;
        virtual void SetErrorPolicy(I2cErrorPolicy& policy) = 0;
        virtual void ResetErrorPolicy() = 0;
    };

    class I2cSlave
    {
    public:
        I2cSlave() = default;
        I2cSlave(const I2cSlave& other) = delete;
        I2cSlave& operator=(const I2cSlave& other) = delete;

    protected:
        ~I2cSlave() = default;

    public:
        virtual void AddSlave(uint8_t ownAddress, infra::Function<void(DataDirection)> onAddressed) = 0;
        virtual void RemoveSlave(uint8_t ownAddress) = 0;

        virtual void SendData(infra::ConstByteRange data, infra::Function<void(Result, uint32_t numberOfBytesSent)> onSent) = 0;
        virtual void ReceiveData(infra::ByteRange data, bool lastOfSession, infra::Function<void(Result, uint32_t numberOfBytesReceived)> onReceived) = 0;

        virtual void StopTransceiving() = 0;
    };
}

#endif
