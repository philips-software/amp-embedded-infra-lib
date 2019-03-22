#ifndef HAL_STUB_FLASH_MOCK_HPP
#define HAL_STUB_FLASH_MOCK_HPP

#include "gmock/gmock.h"
#include "hal/interfaces/Flash.hpp"

namespace hal
{
    //TICS -INT#002: A mock or stub may have public data
    class FlashMock
        : public hal::Flash
    {
    public:
        explicit FlashMock(uint32_t numberOfSectors = 4, uint32_t sizeOfSectors = 16);

        virtual uint32_t NumberOfSectors() const override;
        virtual uint32_t SizeOfSector(uint32_t sectorIndex) const override;

        virtual uint32_t SectorOfAddress(uint32_t address) const override;
        virtual uint32_t AddressOfSector(uint32_t sectorIndex) const override;

        virtual void WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        virtual void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        virtual void EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone) override;

        uint32_t numberOfSectors;
        uint32_t sizeOfSectors;

        MOCK_METHOD2(writeBufferMock, void(std::vector<uint8_t>, uint32_t));
        MOCK_METHOD1(readBufferMock, std::vector<uint8_t>(uint32_t));
        MOCK_METHOD2(eraseSectorsMock, void(uint32_t, uint32_t));

        infra::Function<void()> done;
    };

    class CleanFlashMock
        : public hal::Flash
    {
    public:
        CleanFlashMock() = default;
        CleanFlashMock(uint32_t numberOfSectors, uint32_t sizeOfEachSector);

        MOCK_CONST_METHOD0(NumberOfSectors, uint32_t());
        MOCK_CONST_METHOD1(SizeOfSector, uint32_t(uint32_t sectorIndex));
        MOCK_CONST_METHOD1(SectorOfAddress, uint32_t(uint32_t address));
        MOCK_CONST_METHOD1(AddressOfSector, uint32_t(uint32_t sectorIndex));
        MOCK_METHOD3(WriteBuffer, void(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone));
        MOCK_METHOD3(ReadBuffer, void(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone));
        MOCK_METHOD3(EraseSectors, void(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone));
    };
}

#endif
