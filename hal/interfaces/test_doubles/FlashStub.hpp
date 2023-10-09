#ifndef HAL_STUB_FLASH_STUB_HPP
#define HAL_STUB_FLASH_STUB_HPP

#include "hal/interfaces/Flash.hpp"
#include "infra/util/Optional.hpp"

namespace hal
{
    template<class T>
    class FlashStubBase;

    using FlashStub = FlashStubBase<uint32_t>;
    using FlashStub64 = FlashStubBase<uint64_t>;

    template<class T>
    class FlashStubBase
        : public hal::FlashBase<T>
    {
    public:
        FlashStubBase(T numberOfSectors, uint32_t sizeOfEachSector);

        T NumberOfSectors() const override;
        uint32_t SizeOfSector(T sectorIndex) const override;

        T SectorOfAddress(T address) const override;
        T AddressOfSector(T sectorIndex) const override;

        void WriteBuffer(infra::ConstByteRange buffer, T address, infra::Function<void()> onDone) override;
        void ReadBuffer(infra::ByteRange buffer, T address, infra::Function<void()> onDone) override;
        void EraseSectors(T beginIndex, T endIndex, infra::Function<void()> onDone) override;

    public:
        std::vector<std::vector<uint8_t>> sectors;
        infra::Optional<uint8_t> stopAfterWriteSteps;

        infra::Function<void()> onEraseDone;
        bool delaySignalEraseDone = false;
    };
}

#endif
