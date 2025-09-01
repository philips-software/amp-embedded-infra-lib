#include "services/util/SC16IS7xx.hpp"
#include "hal/interfaces/Gpio.hpp"
#include "hal/interfaces/Spi.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/stream/StreamManipulators.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/ByteRange.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include "services/tracer/Tracer.hpp"
#include <chrono>
#include <cstdint>

namespace services
{
    Sc16Is7xx::Sc16Is7xx(infra::ByteRange bufferStorage, hal::SpiMaster& spi, hal::GpioPin& intPin, const Config& config)
        : txBuffer(bufferStorage)
        , spi(spi)
        , intPin(intPin)
        , config(config)
    // , tracer(services::GlobalSoftTracer())
    {
        intPin.Config(hal::PinConfigType::input);
        intPin.EnableInterrupt([this]()
            {
                infra::EventDispatcher::Instance().Schedule([this]()
                    {
                        HandleInterrupt();
                    });
            },
            hal::InterruptTrigger::fallingEdge);
    }

    void Sc16Is7xx::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        // tracer.Trace() << "Sc16Is7xx::SendData: size " << data.size() << ", buffer available " << txBuffer.Available();
        this->onSendData = actionOnCompletion;
        data.shrink_from_back_to(txBuffer.Available());
        // tracer.Trace() << "Sc16Is7xx::SendData: size " << data.size() << ", buffer available " << txBuffer.Available();
        txBuffer.Push(data);
        this->sendData = true;
    }

    void Sc16Is7xx::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        this->dataReceived = dataReceived;
    }

    void Sc16Is7xx::WriteRegister(uint8_t reg, uint8_t value, infra::Function<void()> onDone)
    {
        this->onWriteRegisterDone = onDone;
        this->data[0] = (reg & 0x0F) << 3;
        this->data[1] = value;
        this->value = value;
        // tracer.Trace() << "Sc16Is7xx::WriteRegister reg 0x" << infra::hex << reg << " value 0x" << infra::hex << value << ", range " << infra::AsHex(this->data);
        spi.SendData(this->data, hal::SpiAction::stop, [this]()
            {
                this->onWriteRegisterDone();
            });
    }

    void Sc16Is7xx::ReadRegister(uint8_t reg, infra::Function<void(uint8_t value)> onDone)
    {
        this->onReadRegisterDone = onDone;
        this->reg = 0;
        this->reg = 0x80 | ((reg & 0x0F) << 3);
        spi.SendData(infra::MakeByteRange(this->reg), hal::SpiAction::continueSession, [this]()
            {
                spi.ReceiveData(infra::MakeByteRange(this->value), hal::SpiAction::stop, [this]()
                    {
                        // tracer.Trace() << "Sc16Is7xx::ReadRegister reg 0x" << infra::hex << ((this->reg & 0x7F) >> 3) << " value 0x" << infra::hex << this->value;
                        this->onReadRegisterDone(this->value);
                    });
            });
    }

    void Sc16Is7xx::WriteFifo(infra::ConstByteRange data, infra::Function<void()> onDone)
    {
        if (data.size() == 0)
        {
            onDone();
            return;
        }
        this->reg = 0x00;
        this->fifoWriteData = data;
        this->onFifoReadWriteDone = onDone;
        // tracer.Trace() << "Sc16Is7xx::WriteFifo " << data.size();
        spi.SendData(infra::MakeByteRange(this->reg), hal::SpiAction::continueSession, [this]()
            {
                spi.SendData(this->fifoWriteData, hal::SpiAction::stop, [this]()
                    {
                        this->onFifoReadWriteDone();
                    });
            });
    }

    void Sc16Is7xx::ReadFifo(infra::ByteRange data, infra::Function<void()> onDone)
    {
        this->reg = 0x80;
        this->fifoReadData = data;
        this->onFifoReadWriteDone = onDone;
        spi.SendData(infra::MakeByteRange(this->reg), hal::SpiAction::continueSession, [this]()
            {
                spi.ReceiveData(this->fifoReadData, hal::SpiAction::stop, [this]()
                    {
                        this->onFifoReadWriteDone();
                    });
            });
    }

    void Sc16Is7xx::Configure(infra::Function<void()> onDone)
    {
        this->onConfigureDone = onDone;

        // tracer.Trace() << "Configure()";
        GetConfiguration();
        WriteRegisters([this]()
            {
                // tracer.Trace() << "WriteRegisters done!";
                ReadRegisters([this]()
                    {
                        // tracer.Trace() << "ReadRegisters done!";
                        interrupt = false;
                        Process();
                        this->onConfigureDone();
                    });
            });
    }

    infra::MemoryRange<Sc16Is7xx::RegisterConfig> Sc16Is7xx::GetConfiguration()
    {
        registerConfiguration.clear();

        // Baudrate
        // DLL DLH: clock division registers
        uint32_t divisor = 0u;
        divisor = CrystalFrequency / (config.baudrate * 16);
        registerConfiguration.push_back(RegisterConfig{ LCR, 0x80 });
        registerConfiguration.push_back(RegisterConfig{ DLH, static_cast<uint8_t>((divisor >> 8) & 0xFF) });
        registerConfiguration.push_back(RegisterConfig{ DLL, static_cast<uint8_t>(divisor & 0xFF) });
        registerConfiguration.push_back(RegisterConfig{ LCR, 0xBF });

        // Enhanced registers
        registerConfiguration.push_back(RegisterConfig{ EFR, 0x00 });

        // LCR: Parity, stop bits, word length
        uint8_t lcr = 0;
        // Parity
        if (config.parity == detail::ParityDisable)
            lcr |= ParityDisable;
        if (config.parity == detail::ParityOdd)
            lcr |= ParityOdd;
        if (config.parity == detail::ParityEven)
            lcr |= ParityEven;
        if (config.parity == detail::ParityForced0)
            lcr |= ParityForced0;
        if (config.parity == detail::ParityForced1)
            lcr |= ParityForced1;
        // word length
        lcr |= WordLength8Bits;
        // stop bits
        lcr |= StopBits1;
        registerConfiguration.push_back(RegisterConfig{ LCR, lcr });

        // FCR: FIFO control register
        registerConfiguration.push_back(RegisterConfig{ FCR, EnableFifo | TxFifoReset | RxFifoReset });

        // SPR: scratch pad
        registerConfiguration.push_back(RegisterConfig{ SPR, 'A' });

        // loopback + TLR
        registerConfiguration.push_back(RegisterConfig{ MCR, 0x04 });

        // TLR
        registerConfiguration.push_back(RegisterConfig{ TLR, 0x10 | 0x01 });

        // IER: interrupts
        registerConfiguration.push_back(RegisterConfig{ IER, TxHoldingRegisterInterruptEnable | RxHoldingRegisterInterruptEnable });

        // tracer.Trace() << "GetConfiguration() -> computed configuration: ";
        for (const auto& e : registerConfiguration)
        {
            ; // tracer.Trace() << "reg 0x" << infra::hex << e.reg << " : 0x" << infra::hex << e.value;
        }
        return registerConfiguration.range();
    }

    void Sc16Is7xx::Process()
    {
        seq.Load([this]()
            {
                seq.While([this]()
                    {
                        return true;
                    });

                seq.If([this]()
                    {
                        return interrupt || !intPin.Get();
                    });

                seq.Step([this]()
                    {
                        interrupt = false;
                        // Read interrupt status register
                        ReadRegister(IIR, [this](uint8_t value)
                            {
                                interruptState = value;
                                // tracer.Trace() << "IIR 0x" << infra::hex << interruptState;
                                this->seq.Continue();
                            });
                    });
                seq.Step([this]()
                    {
                        if (interruptState & (RxRHRInterrupt | RxTimeoutInterrupt))
                        {
                            // tracer.Trace() << "Process rx interrupt";
                            ProcessRxInterrupt([this]()
                                {
                                    this->seq.Continue();
                                });
                        }
                    });

                seq.Step([this]()
                    {
                        if (interruptState & (TxTHRInterrupt))
                        {
                            // tracer.Trace() << "Process tx interrupt";
                            this->txBusy = false;
                            ProcessTxInterrupt([this]()
                                {
                                    this->seq.Continue();
                                });
                        }
                    });

                seq.Step([this]()
                    {
                        if (interruptState & RxLineStatusError)
                        {
                            // tracer.Trace() << "Process error interrupt";
                            ProcessErrorInterrupt([this]()
                                {
                                    this->seq.Continue();
                                });
                        }
                    });
                seq.Execute([this]()
                    {
                        interruptState = 0u;
                    });
                seq.ElseIf([this]()
                    {
                        return !this->txBuffer.Empty() && !this->txBusy && this->sendData;
                    });

                seq.Step([this]()
                    {
                        // tracer.Trace() << "Queueing tx data...";
                        ProcessTxInterrupt([this]()
                            {
                                this->seq.Continue();
                            });
                    });
                seq.EndIf();

                seq.Step([this]()
                    {
                        this->intTimer.Start(std::chrono::milliseconds(20), [this]()
                            {
                                this->seq.Continue();
                            });
                    });
                seq.EndWhile();
            });
    }

    void Sc16Is7xx::HandleInterrupt()
    {
        interrupt = true;
        // tracer.Trace() << "INT";
    }

    void Sc16Is7xx::ProcessRxInterrupt(infra::Function<void()> onDone)
    {
        this->onInterruptProcessDone = onDone;

        // read out all data from FIFO
        ReadRegister(RXLVL, [this](uint8_t value)
            {
                // tracer.Trace() << "RX FIFO size: " << value;
                this->fifoReadData = infra::ByteRange(fifoData.begin(), fifoData.begin() + value);
                ReadFifo(this->fifoReadData, [this]()
                    {
                        infra::EventDispatcher::Instance().Schedule([this]()
                            {
                                if (this->dataReceived != nullptr)
                                {
                                    this->dataReceived(this->fifoReadData);
                                }
                            });
                        // tracer.Trace() << "RX FIFO data: " << infra::AsHex(this->fifoReadData);
                        this->onInterruptProcessDone();
                    });
            });
    }

    void Sc16Is7xx::ProcessTxInterrupt(infra::Function<void()> onDone)
    {
        this->onInterruptProcessDone = onDone;

        if (this->txBuffer.Empty())
        {
            if (this->onSendData != nullptr)
            {
                this->onSendData();
            }
            infra::EventDispatcher::Instance().Schedule([this]()
                {
                    this->onInterruptProcessDone();
                });
        }
        else
        {
            this->ReadRegister(TXLVL, [this](uint8_t sizeAvailable)
                {
                    auto size = std::min<uint8_t>(sizeAvailable, std::min<size_t>(64u, this->txBuffer.Size()));

                    // this->tracer.Trace() << "ProcessTxInterrupt: txBuffer " << this->txBuffer.Size() << ", TXLVL " << sizeAvailable << ", size " << size;

                    auto range = this->txBuffer.ContiguousRange();
                    // this->tracer.Trace() << "ProcessTxInterrupt: range " << range.size();
                    range.shrink_from_back_to(size);

                    // this->tracer.Trace() << "ProcessTxInterrupt: range " << range.size();

                    this->WriteFifo(range, [this, size]()
                        {
                            this->txBusy = true;
                            this->txBuffer.Pop(size);
                            // this->tracer.Trace() << "ProcessTxInterrupt: done, txbuffer size " << this->txBuffer.Size();
                            this->onInterruptProcessDone();
                        });
                });
        }
    }

    void Sc16Is7xx::ProcessErrorInterrupt(infra::Function<void()> onDone)
    {
        this->onInterruptProcessDone = onDone;
        infra::EventDispatcher::Instance().Schedule([this]()
            {
                this->onInterruptProcessDone();
            });
    }

    void Sc16Is7xx::ReadRegisters(infra::Function<void()> onDone)
    {
        this->onReadDone = onDone;

        // tracer.Trace() << "ReadRegisters()";

        this->readReg = 0x00;
        seqRead.Load([this]()
            {
                this->seqRead.While([this]()
                    {
                        return this->readReg <= 0x0F;
                    });

                this->seqRead.Step([this]()
                    {
                        this->ReadRegister(this->readReg, [this](uint8_t value)
                            {
                                // tracer.Trace() << "reg 0x" << infra::hex << this->readReg << " : 0x" << infra::hex << value;
                                this->readReg++;
                                this->seqRead.Continue();
                            });
                    });
                this->seqRead.EndWhile();

                this->seqRead.Execute([this]()
                    {
                        // tracer.Trace() << "reading reg completed";
                        this->onReadDone();
                    });
            });
    }

    void Sc16Is7xx::WriteRegisters(infra::Function<void()> onDone)
    {
        this->onWriteDone = onDone;

        // tracer.Trace() << "WriteRegisters()";

        registerConfig = registerConfiguration.range();
        seqWrite.Load([this]()
            {
                this->seqWrite.While([this]()
                    {
                        return !this->registerConfig.empty();
                    });

                this->seqWrite.Step([this]()
                    {
                        const auto& entry = this->registerConfig.front();
                        // tracer.Trace() << "writting reg 0x" << infra::hex << entry.reg << " : 0x" << infra::hex << entry.value;
                        this->WriteRegister(entry.reg, entry.value, [this]()
                            {
                                this->registerConfig.pop_front();
                                this->seqWrite.Continue();
                            });
                    });
                this->seqWrite.EndWhile();

                this->seqWrite.Execute([this]()
                    {
                        // tracer.Trace() << "writting reg completed";
                        this->onWriteDone();
                    });
            });
    }
};
