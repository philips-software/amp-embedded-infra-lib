#ifndef SC16IS7XX_DRIVER_H
#define SC16IS7XX_DRIVER_H

#include "hal/interfaces/Gpio.hpp"
#include "hal/interfaces/SerialCommunication.hpp"
#include "hal/interfaces/Spi.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/CyclicBuffer.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/MemoryRange.hpp"
#include "infra/util/Sequencer.hpp"
#include "services/tracer/Tracer.hpp"
#include <array>
#include <cstdint>

namespace services
{
    namespace detail
    {
        enum Parity
        {
            ParityDisable = 0,
            ParityOdd,
            ParityEven,
            ParityForced1,
            ParityForced0
        };

        struct UartTiConfig
        {
            uint32_t baudrate{ 115200 };
            Parity parity{ ParityDisable };
            bool flowControl{ false };
        };

        struct RegisterConfiguration
        {
            uint8_t reg;
            uint8_t value;
        };
    }

    class Sc16Is7xx
        : public hal::SerialCommunication
    {
    public:
        using Parity = detail::Parity;
        using Config = detail::UartTiConfig;
        using RegisterConfig = detail::RegisterConfiguration;

        enum class Interrupt : uint8_t
        {
            ReceiverLineStatus = 0x01,
            RxTimeout = 0x02,
            Rx = 0x04,
            TxEmpty = 0x08,
            ModemPinStatus = 0x10,
            IoPins = 0x20,
            RxXoff = 0x40,
            RtsCtsPinState = 0x80
        };

        static constexpr uint32_t CrystalFrequency = 1843200;

    public:
        template<std::size_t StorageSize>
        using WithStorage = infra::WithStorage<Sc16Is7xx, std::array<uint8_t, StorageSize>>;
        Sc16Is7xx(infra::ByteRange bufferStorage, hal::SpiMaster& spi, hal::GpioPin& intPin, const Config& config);

        // hal::SerialCommunication
        void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
        void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;

        void Configure(infra::Function<void()> onDone);
        infra::MemoryRange<RegisterConfig> GetConfiguration();

    private:
        infra::CyclicByteBuffer txBuffer;
        hal::SpiMaster& spi;
        hal::GpioPin& intPin;
        const Config& config;

        infra::Function<void(infra::ConstByteRange data)> dataReceived;
        infra::AutoResetFunction<void()> onSendData;

        const uint8_t RHR = 0x00;
        const uint8_t THR = 0x00;
        const uint8_t IER = 0x01;
        const uint8_t FCR = 0x02;
        const uint8_t IIR = 0x02;
        const uint8_t LCR = 0x03;
        const uint8_t MCR = 0x04;
        const uint8_t LSR = 0x05;
        const uint8_t MSR = 0x06;
        const uint8_t SPR = 0x07;
        const uint8_t TCR = 0x06;
        const uint8_t TLR = 0x07;
        const uint8_t TXLVL = 0x08;
        const uint8_t RXLVL = 0x09;
        const uint8_t EFCR = 0x0F;

        /* Special register set LCR[7] = 1 */
        const uint8_t DLL = 0x00;
        const uint8_t DLH = 0x01;
        /* Enhanced register set LCR   = 0xBF.*/
        const uint8_t EFR = 0x02;
        const uint8_t XON1 = 0x04;
        const uint8_t XON2 = 0x05;
        const uint8_t XOFF1 = 0x06;
        const uint8_t XOFF2 = 0x07;

        // FCR
        enum RxFifoSize : uint8_t
        {
            RxFifoTrigger8Bytes = 0b00000000,
            RxFifoTrigger16Bytes = 0b01000000,
            RxFifoTrigger56Bytes = 0b10000000,
            RxFifoTrigger60Bytes = 0b11000000,
        };

        enum TxFifoSize : uint8_t
        {
            TxFifoTrigger8Bytes = 0b00000000,
            TxFifoTrigger16Bytes = 0b00010000,
            TxFifoTrigger32Bytes = 0b00100000,
            TxFifoTrigger56Bytes = 0b00110000,
        };

        static constexpr uint8_t TxFifoReset = 0b00000100;
        static constexpr uint8_t RxFifoReset = 0b00000010;
        static constexpr uint8_t EnableFifo = 0b00000001;

        // LCR
        static constexpr uint8_t ParityDisable = 0b00000000;
        static constexpr uint8_t ParityOdd = 0b00011000;
        static constexpr uint8_t ParityEven = 0b00011000;
        static constexpr uint8_t ParityForced1 = 0b00101000;
        static constexpr uint8_t ParityForced0 = 0b00111000;

        static constexpr uint8_t StopBits1 = 0b00000000;
        static constexpr uint8_t StopBits1AndHalfOr2 = 0b00000100;

        static constexpr uint8_t WordLength5Bits = 0b00000000;
        static constexpr uint8_t WordLength6Bits = 0b00000001;
        static constexpr uint8_t WordLength7Bits = 0b00000010;
        static constexpr uint8_t WordLength8Bits = 0b00000011;

        // LSR
        static constexpr uint8_t FifoDataError = 0b10000000;
        static constexpr uint8_t TransmitEmpty = 0b01000000;
        static constexpr uint8_t TransmitHoldingRegisterEmpty = 0b00100000;
        static constexpr uint8_t BreakInterrupt = 0b00010000;
        static constexpr uint8_t FramingError = 0b00001000;
        static constexpr uint8_t ParityError = 0b00000100;
        static constexpr uint8_t OverrunError = 0b00000010;
        static constexpr uint8_t DataInReceiver = 0b00000001;

        // IER
        static constexpr uint8_t CtsInterruptEnable = 0b10000000;
        static constexpr uint8_t RtsInterruptEnable = 0b01000000;
        static constexpr uint8_t XoffInterruptEnable = 0b00100000;
        static constexpr uint8_t SleepInterruptEnable = 0b00010000;
        static constexpr uint8_t ModemStatusInterruptEnable = 0b00001000;
        static constexpr uint8_t ReceiverLineStatusInterruptEnable = 0b00000100;
        static constexpr uint8_t TxHoldingRegisterInterruptEnable = 0b00000010;
        static constexpr uint8_t RxHoldingRegisterInterruptEnable = 0b00000001;

        // IIR
        static constexpr uint8_t RxLineStatusError = 0b00000110;
        static constexpr uint8_t RxTimeoutInterrupt = 0b00001100;
        static constexpr uint8_t RxRHRInterrupt = 0b00000100;
        static constexpr uint8_t TxTHRInterrupt = 0b00000010;
        static constexpr uint8_t ModemInterrupt = 0b00000000;
        static constexpr uint8_t PinChangeInterrupt = 0b00110000;
        static constexpr uint8_t XOffInterrupt = 0b00010000;
        static constexpr uint8_t CtsRtsActiveToInactiveInterrupt = 0b00100000;

        // EFR
        static constexpr uint8_t CTSFlowControlEnable = 0b10000000;
        static constexpr uint8_t RTSFlowControlEnable = 0b01000000;
        static constexpr uint8_t EnhancedFunctionEnable = 0b00010000;

        void WriteRegister(uint8_t reg, uint8_t value, infra::Function<void()> onDone);
        void ReadRegister(uint8_t reg, infra::Function<void(uint8_t value)> onDone);

        void WriteFifo(infra::ConstByteRange data, infra::Function<void()> onDone);
        void ReadFifo(infra::ByteRange data, infra::Function<void()> onDone);
        infra::Function<void()> onFifoReadWriteDone;

        infra::Function<void()> onWriteRegisterDone;
        infra::Function<void(uint8_t value)> onReadRegisterDone;

        uint8_t reg{};
        uint8_t value{};
        uint8_t readReg{};
        uint8_t readValue{};

        std::array<uint8_t, 2> data{};
        std::array<uint8_t, 64u> fifoData{};
        infra::ByteRange fifoReadData;
        infra::ConstByteRange fifoWriteData;

        infra::Function<void()> onConfigureDone;

        infra::BoundedVector<RegisterConfig>::WithMaxSize<16> registerConfiguration;
        infra::MemoryRange<RegisterConfig> registerConfig;

        void HandleInterrupt();

        volatile bool interrupt{};
        uint8_t interruptState{};
        bool rxInterrupt{};
        bool txEmptyInterrupt{};
        bool errorInterrupt{};

        void ProcessRxInterrupt(infra::Function<void()> onDone);
        void ProcessTxInterrupt(infra::Function<void()> onDone);
        void ProcessErrorInterrupt(infra::Function<void()> onDone);

        infra::Function<void()> onInterruptProcessDone;

        void ReadRegisters(infra::Function<void()> onDone);
        infra::Function<void()> onReadDone;
        infra::Sequencer seqRead;

        void WriteRegisters(infra::Function<void()> onDone);
        infra::Function<void()> onWriteDone;
        infra::Sequencer seqWrite;
        void Process();
        infra::Sequencer seq;

        std::array<uint8_t, 4u> txData{};

        infra::TimerRepeating intTimer;
        bool txBusy = false;

        infra::TimerSingleShot sendTimer;
        bool sendData = false;

        // services::Tracer& tracer;
    };
}
#endif
