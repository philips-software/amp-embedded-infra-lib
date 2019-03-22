#ifndef SERVICES_I2C_MULTIPLE_ACCESS_HPP
#define SERVICES_I2C_MULTIPLE_ACCESS_HPP

#include "hal/interfaces/I2c.hpp"
#include "infra/event/ClaimableResource.hpp"
#include "infra/util/AutoResetFunction.hpp"

#ifndef SERVICES_I2C_MULTIPLE_ACCESS_FUNCTION_EXTRA_SIZE                                                        //TICS !POR#021
#define SERVICES_I2C_MULTIPLE_ACCESS_FUNCTION_EXTRA_SIZE (INFRA_DEFAULT_FUNCTION_EXTRA_SIZE + (8 * sizeof(void*)))
#endif

namespace services
{
    class I2cMultipleAccessMaster                                                                               //TICS !OOP#013
        : public hal::I2cMaster
        , public infra::ClaimableResource
    {
    public:
        explicit I2cMultipleAccessMaster(hal::I2cMaster& master);

        virtual void SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent) override;
        virtual void ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result)> onReceived) override;

    private:
        hal::I2cMaster& master;
    };

    class I2cMultipleAccess
        : public hal::I2cMaster
    {
    public:
        explicit I2cMultipleAccess(I2cMultipleAccessMaster& master);

        virtual void SendData(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result, uint32_t numberOfBytesSent)> onSent) override;
        virtual void ReceiveData(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction,
            infra::Function<void(hal::Result)> onReceived) override;

    private:
        void SendDataOnClaimed(hal::I2cAddress address, infra::ConstByteRange data, hal::Action nextAction);
        void ReceiveDataOnClaimed(hal::I2cAddress address, infra::ByteRange data, hal::Action nextAction);

    private:
        I2cMultipleAccessMaster& master;
        infra::ClaimableResource::Claimer::WithSize<SERVICES_I2C_MULTIPLE_ACCESS_FUNCTION_EXTRA_SIZE> claimer;
        infra::AutoResetFunction<void(hal::Result, uint32_t numberOfBytesSent)> onSent;
        infra::AutoResetFunction<void(hal::Result)> onReceived;
    };
}

#endif
