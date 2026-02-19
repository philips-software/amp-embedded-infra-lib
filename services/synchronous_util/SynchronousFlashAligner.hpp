#ifndef SERVICES_SYNCHRONOUS_FLASH_ALIGNER_HPP
#define SERVICES_SYNCHRONOUS_FLASH_ALIGNER_HPP

#include "hal/synchronous_interfaces/SynchronousFlash.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/synchronous_util/SynchronousFlashDelegate.hpp"
#include <optional>

namespace services
{
    // The purpose of this class is to make sure that writes to flash are always aligned on a certain boundary. This is used
    // when writing to internal flash, which must often be written in multiples of 8 or 16 bytes
    class SynchronousFlashAligner
        : public SynchronousFlashDelegateBase
    {
    public:
        template<std::size_t Alignment>
        using WithAlignment = infra::WithStorage<SynchronousFlashAligner, infra::BoundedVector<uint8_t>::WithMaxSize<Alignment>>;

        SynchronousFlashAligner(infra::BoundedVector<uint8_t>& buffer, hal::SynchronousFlash& delegate);

        void Flush();

        void WriteBuffer(infra::ConstByteRange buffer, uint32_t address) override;
        void ReadBuffer(infra::ByteRange buffer, uint32_t address) override;
        void EraseSectors(uint32_t beginIndex, uint32_t endIndex) override;

    private:
        bool OverlapsWithBufferedData(uint32_t rangeStart, uint32_t rangeEnd) const;

    private:
        infra::BoundedVector<uint8_t>& alignedBuffer_;
        std::optional<uint32_t> address_{ std::nullopt };
    };
}

#endif
