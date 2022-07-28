#ifndef SERVICES_SPI_MULTIPLE_ACCESS_HPP
#define SERVICES_SPI_MULTIPLE_ACCESS_HPP

#include "hal/interfaces/Spi.hpp"
#include "infra/event/ClaimableResource.hpp"
#include "infra/util/AutoResetFunction.hpp"

#ifndef SERVICES_SPI_MULTIPLE_ACCESS_FUNCTION_EXTRA_SIZE
#define SERVICES_SPI_MULTIPLE_ACCESS_FUNCTION_EXTRA_SIZE (INFRA_DEFAULT_FUNCTION_EXTRA_SIZE + (7 * sizeof(void*)))
#endif

namespace services
{
    class SpiMultipleAccessMaster
        : public hal::SpiMaster
        , public infra::ClaimableResource
    {
    public:
        explicit SpiMultipleAccessMaster(hal::SpiMaster& master);

        virtual void SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, hal::SpiAction nextAction, const infra::Function<void()>& onDone) override;
        virtual void SetChipSelectConfigurator(hal::ChipSelectConfigurator& configurator) override;
        virtual void SetCommunicationConfigurator(hal::CommunicationConfigurator& configurator) override;
        virtual void ResetCommunicationConfigurator() override;

    private:
        hal::SpiMaster& master;
        hal::CommunicationConfigurator* communicationConfigurator = nullptr;
    };

    class SpiMultipleAccess
        : public hal::SpiMaster
        , private hal::ChipSelectConfigurator
    {
    public:
        explicit SpiMultipleAccess(SpiMultipleAccessMaster& master);

        virtual void SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, hal::SpiAction nextAction, const infra::Function<void()>& onDone) override;
        virtual void SetChipSelectConfigurator(hal::ChipSelectConfigurator& configurator) override; 
        virtual void SetCommunicationConfigurator(hal::CommunicationConfigurator& configurator) override;
        virtual void ResetCommunicationConfigurator() override;

    private:
        virtual void StartSession() override;
        virtual void EndSession() override;
        void SendAndReceiveOnClaimed(infra::ConstByteRange sendData, infra::ByteRange receiveData, hal::SpiAction nextAction, const infra::Function<void()>& onDone);

    private:
        SpiMultipleAccessMaster& master;
        infra::ClaimableResource::Claimer::WithSize<SERVICES_SPI_MULTIPLE_ACCESS_FUNCTION_EXTRA_SIZE> claimer;
        hal::ChipSelectConfigurator* chipSelectConfigurator = nullptr;
        hal::CommunicationConfigurator* communicationConfigurator = nullptr;
    };
}

#endif
