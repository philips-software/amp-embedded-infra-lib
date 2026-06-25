#ifndef SERVICES_TRACING_FLASH_HPP
#define SERVICES_TRACING_FLASH_HPP

#include "hal/interfaces/Flash.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/BoundedString.hpp"
#include "services/tracer/Tracer.hpp"

namespace services
{
    class TracingFlash
        : public hal::Flash
    {
    public:
        TracingFlash(infra::BoundedConstString name, hal::Flash& flash, services::Tracer& tracer);

        uint32_t NumberOfSectors() const override;
        uint32_t SizeOfSector(uint32_t sectorIndex) const override;
        uint32_t SectorOfAddress(uint32_t address) const override;
        uint32_t AddressOfSector(uint32_t sectorIndex) const override;

        void WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        void EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone) override;

    private:
        infra::BoundedConstString name;
        hal::Flash& flash;
        services::Tracer& tracer;

        infra::AutoResetFunction<void()> onDone;
        infra::ConstByteRange buffer;
    };
}

#endif
