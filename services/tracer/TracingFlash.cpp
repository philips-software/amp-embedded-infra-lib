#include "services/tracer/TracingFlash.hpp"
#include "infra/util/BoundedString.hpp"

namespace services
{
    TracingFlash::TracingFlash(infra::BoundedConstString name, hal::Flash& flash, services::Tracer& tracer)
        : name(name)
        , flash(flash)
        , tracer(tracer)
    {}

    uint32_t TracingFlash::NumberOfSectors() const
    {
        return flash.NumberOfSectors();
    }

    uint32_t TracingFlash::SizeOfSector(uint32_t sectorIndex) const
    {
        return flash.SizeOfSector(sectorIndex);
    }

    uint32_t TracingFlash::SectorOfAddress(uint32_t address) const
    {
        return flash.SectorOfAddress(address);
    }

    uint32_t TracingFlash::AddressOfSector(uint32_t sectorIndex) const
    {
        return flash.AddressOfSector(sectorIndex);
    }

    void TracingFlash::WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        tracer.Trace() << name << " Flash WriteBuffer to 0x" << infra::hex << address; //<< ": " << infra::AsHex(buffer);
        flash.WriteBuffer(buffer, address, onDone);
    }

    void TracingFlash::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        tracer.Trace() << name << " Flash ReadBuffer from 0x" << infra::hex << address << " of size " << buffer.size();
        this->onDone = onDone;
        this->buffer = buffer;
        flash.ReadBuffer(buffer, address, [this, address]()
            {
                // tracer.Trace() << name << " Flash ReadBuffer from 0x" << infra::hex << address << ": " << infra::AsHex(this->buffer);
                this->onDone();
            });
    }

    void TracingFlash::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        tracer.Trace() << name << " Flash EraseSectors 0x" << infra::hex << beginIndex << " to 0x" << infra::hex << endIndex;
        flash.EraseSectors(beginIndex, endIndex, onDone);
    }
}
