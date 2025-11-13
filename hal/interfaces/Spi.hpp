#ifndef HAL_SPI_HPP
#define HAL_SPI_HPP

#include "hal/interfaces/CommunicationConfigurator.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Observer.hpp"

namespace hal
{
    enum SpiAction
    {
        continueSession,
        stop
    };

    class ChipSelectConfigurator
    {
    protected:
        ChipSelectConfigurator() = default;
        ChipSelectConfigurator(const ChipSelectConfigurator& other) = delete;
        ChipSelectConfigurator& operator=(const ChipSelectConfigurator& other) = delete;
        ~ChipSelectConfigurator() = default;

    public:
        virtual void StartSession() = 0;
        virtual void EndSession() = 0;
    };

    class SpiMaster
    {
    protected:
        SpiMaster() = default;
        SpiMaster(const SpiMaster& other) = delete;
        SpiMaster& operator=(const SpiMaster& other) = delete;
        ~SpiMaster() = default;

    public:
        void SendData(infra::ConstByteRange data, SpiAction nextAction, const infra::Function<void()>& onDone);
        void ReceiveData(infra::ByteRange data, SpiAction nextAction, const infra::Function<void()>& onDone);
        virtual void SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, SpiAction nextAction, const infra::Function<void()>& onDone) = 0;
        virtual void SetChipSelectConfigurator(ChipSelectConfigurator& configurator) = 0;
        virtual void SetCommunicationConfigurator(CommunicationConfigurator& configurator) = 0;
        virtual void ResetCommunicationConfigurator() = 0;
    };

    class ChipSelectObserver
        : public infra::SingleObserver<ChipSelectObserver, class ChipSelectSubject>
    {
    public:
        using infra::SingleObserver<ChipSelectObserver, ChipSelectSubject>::SingleObserver;

        virtual void OnSelectedOnInterrupt() = 0;
        virtual void OnDeselectedOnInterrupt() = 0;
    };

    class ChipSelectSubject
        : public infra::Subject<ChipSelectObserver>
    {
    public:
        virtual void EnableChipSelectInterrupt() = 0;
        virtual void DisableChipSelectInterrupt() = 0;
    };

    class SpiSlave
    {
    protected:
        SpiSlave() = default;
        SpiSlave(const SpiSlave& other) = delete;
        SpiSlave& operator=(const SpiSlave& other) = delete;
        ~SpiSlave() = default;

    public:
        virtual void SendAndReceive(infra::ConstByteRange sendData, infra::ByteRange receiveData, const infra::Function<void()>& onDone) = 0;
        virtual bool CancelTransmission() = 0;
    };
}

#endif
