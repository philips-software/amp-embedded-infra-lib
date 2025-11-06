#ifndef SERVICES_FLASH_MULTIPLE_ACCESS_HPP
#define SERVICES_FLASH_MULTIPLE_ACCESS_HPP

#include "hal/interfaces/Flash.hpp"
#include "infra/event/ClaimableResource.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/ByteRange.hpp"
#include "services/util/FlashDelegate.hpp"

#ifndef SERVICES_FLASH_MULTIPLE_ACCESS_FUNCTION_EXTRA_SIZE
#define SERVICES_FLASH_MULTIPLE_ACCESS_FUNCTION_EXTRA_SIZE (sizeof(services::FlashMultipleAccess::LargestLambdaCapture))
#endif

namespace services
{
    class FlashMultipleAccessMaster
        : public FlashDelegate
        , public infra::ClaimableResource
    {
    public:
        using FlashDelegate::FlashDelegate;
    };

    class FlashMultipleAccess
        : public hal::Flash
    {
    public:
        explicit FlashMultipleAccess(FlashMultipleAccessMaster& master);

        uint32_t NumberOfSectors() const override;
        uint32_t SizeOfSector(uint32_t sectorIndex) const override;
        uint32_t SectorOfAddress(uint32_t address) const override;
        uint32_t AddressOfSector(uint32_t sectorIndex) const override;
        void WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        void EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone) override;

    private:
        FlashMultipleAccessMaster& master;

        struct LargestLambdaCapture
        {
            void* thisPtr;
            infra::ByteRange buffer;
            uint32_t address;
        };

        infra::ClaimableResource::Claimer::WithSize<SERVICES_FLASH_MULTIPLE_ACCESS_FUNCTION_EXTRA_SIZE> claimer;
        infra::AutoResetFunction<void()> onDone;
    };
}

#endif
