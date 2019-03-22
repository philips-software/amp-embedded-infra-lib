#ifndef SERVICES_FLASH_DELAYED_INIT_HPP
#define SERVICES_FLASH_DELAYED_INIT_HPP

#include "hal/interfaces/Flash.hpp"
#include "infra/util/AutoResetFunction.hpp"

#ifndef SERVICES_FLASH_DELAYED_INIT_FUNCTION_EXTRA_SIZE                                                         //TICS !POR#021
#define SERVICES_FLASH_DELAYED_INIT_FUNCTION_EXTRA_SIZE (INFRA_DEFAULT_FUNCTION_EXTRA_SIZE + (5 * sizeof(void*)))
#endif

namespace services
{
    class FlashDelayedInit
        : public hal::Flash
    {
    public:
        explicit FlashDelayedInit(hal::Flash& master);

    public:
        virtual uint32_t NumberOfSectors() const override;
        virtual uint32_t SizeOfSector(uint32_t sectorIndex) const override;

        virtual uint32_t SectorOfAddress(uint32_t address) const override;
        virtual uint32_t AddressOfSector(uint32_t sectorIndex) const override;

        virtual void WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        virtual void ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone) override;
        virtual void EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone) override;

        void Initialized();

    private:
        hal::Flash& master;
        bool initialized = false;

        infra::AutoResetFunction<void(), SERVICES_FLASH_DELAYED_INIT_FUNCTION_EXTRA_SIZE> onInitialized;
    };
}

#endif
