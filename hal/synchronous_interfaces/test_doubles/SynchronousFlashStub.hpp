#ifndef SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_STUB_HPP
#define SYNCHRONOUS_HAL_SYNCHRONOUS_FLASH_STUB_HPP

#include "hal/synchronous_interfaces/SynchronousFlash.hpp"
#include "infra/util/Optional.hpp"

namespace hal
{
    class SynchronousFlashStub
        : public hal::SynchronousFlash
    {
    public:
        SynchronousFlashStub(uint32_t numberOfSectors, uint32_t sizeOfEachSector);

        void Clear();

        uint32_t NumberOfSectors() const override;
        uint32_t SizeOfSector(uint32_t sectorIndex) const override;

        uint32_t SectorOfAddress(uint32_t address) const override;
        uint32_t AddressOfSector(uint32_t sectorIndex) const override;

        void WriteBuffer(infra::ConstByteRange buffer, uint32_t address) override;
        void ReadBuffer(infra::ByteRange buffer, uint32_t address) override;
        void EraseSectors(uint32_t beginIndex, uint32_t endIndex) override;

    private:
        void WriteBufferImpl(infra::ConstByteRange buffer, uint32_t address);
        void ApplyBuffer(infra::ConstByteRange buffer, uint32_t address, uint32_t size);
        void ReadBufferPart(infra::ByteRange& buffer, uint32_t& address);

    public:
        std::vector<std::vector<uint8_t>> sectors;
        std::optional<uint8_t> stopAfterWriteSteps;
    };
}

#endif
