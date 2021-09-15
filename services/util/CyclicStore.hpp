#ifndef SERVICES_CYCLIC_STORE_HPP
#define SERVICES_CYCLIC_STORE_HPP

#include "hal/interfaces/Flash.hpp"
#include "infra/event/ClaimableResource.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
#include "infra/util/Sequencer.hpp"

namespace services
{
    class CyclicStore
    {
    public:
        class Iterator;

        explicit CyclicStore(hal::Flash& flash);
        CyclicStore(const CyclicStore& other) = delete;
        CyclicStore& operator=(const CyclicStore& other) = delete;

        void Add(infra::ConstByteRange range, const infra::Function<void()>& onDone);
        void AddPartial(infra::ConstByteRange range, uint32_t totalSize, const infra::Function<void()>& onDone);
        void Clear(const infra::Function<void()>& onDone);
        void ClearUrgent(const infra::Function<void()>& onDone);

        Iterator Begin() const;

    private:
        void AddClaimed(infra::ConstByteRange range);
        void ClearClaimed();

        void Recover();
        void RecoverSector(uint32_t sectorIndex);
        void RecoverEndAddress();

        void EraseSectorIfAtStart();
        void FillSectorIfDataDoesNotFit(std::size_t size);
        void WriteSectorStatusIfAtStartOfSector();
        void WriteRange(infra::ConstByteRange range);

    private:
        using Length = uint16_t;

        enum class BlockStatus : uint8_t
        {
            empty = 0xff,
            emptyUntilEnd = 0x7f,
            writingLength = 0xfe,
            writingData = 0xfc,
            dataReady = 0xf8,
            erased = 0xf0
        };

        enum class SectorStatus : uint8_t
        {
            empty = 0xff,
            used = 0xfe,
            firstInCycle = 0xfc
        };

        struct BlockHeader
        {
            BlockStatus status = BlockStatus::empty;
            uint8_t lengthLsb = 0;
            uint8_t lengthMsb = 0;

            Length BlockLength() const
            {
                return lengthLsb + static_cast<Length>(lengthMsb << 8);
            }

            void SetBlockLength(Length length)
            {
                lengthLsb = static_cast<uint8_t>(length);
                lengthMsb = static_cast<uint8_t>(length >> 8);
            }
        };

    public:
        class Iterator
            : public infra::IntrusiveForwardList<Iterator>::NodeType
        {
        public:
            explicit Iterator(const CyclicStore& store);
            Iterator(const Iterator& other);
            ~Iterator();
            Iterator& operator=(const Iterator& other);

            void Read(infra::ByteRange buffer, const infra::Function<void(infra::ByteRange result)>& onDone);
            void ErasePrevious(const infra::Function<void()>& onDone); // Erase the item that just hase been read

            void SectorIsErased(uint32_t sectorIndex);

            bool operator==(const Iterator& other) const;

        private:
            void ReadSectorStatusIfAtStart();
            void ReadBlockHeader();
            void ReadData();
            void ErasePreviousData();
            void IncreaseAddressForNonData();

        private:
            const CyclicStore& store;
            bool loadStartAddressDelayed;
            uint32_t address;
            infra::Sequencer sequencer;
            infra::ClaimableResource::Claimer::WithSize<4 * sizeof(void*)> claimer;

            SectorStatus sectorStatus = SectorStatus::used;
            bool firstSectorToRead = true;
            BlockHeader blockHeader;
            infra::ByteRange readBuffer;

            bool found = false;
            bool reachedEnd = false;

            bool previousErased = true; // When the iterator is constructed, it is pointing at the start. Since no previous item exists, it does not need to be erased
            uint32_t addressPreviousBlockHeader;
        };

    private:
        hal::Flash& flash;
        uint32_t startAddress = 0;
        uint32_t endAddress = 0;
        infra::Sequencer sequencer;
        uint32_t sectorIndex = 0;
        mutable infra::ClaimableResource resource;

        infra::ClaimableResource::Claimer::WithSize<4 * sizeof(void*)> claimerAdd;
        infra::ClaimableResource::Claimer::WithSize<4 * sizeof(void*)> claimerClear;
        infra::ClaimableResource::Claimer::WithSize<4 * sizeof(void*)> claimerRecover;
        infra::AutoResetFunction<void()> onAddDone;
        infra::AutoResetFunction<void()> onClearDone;
        uint32_t remainingPartialSize = 0;
        uint32_t partialSizeWritten = 0;
        bool partialAddStarted = false;

        infra::Optional<Iterator> erasingPosition;

        enum class RecoverPhase : uint8_t
        {
            searchingStartOrEmpty,
            checkingAllUsed,
            checkingUsedFollowedByEmpty,
            checkingAllEmpty,
            checkingAllUsedOrAllEmpty,
            corrupt,
            done
        };

        SectorStatus sectorStatus = SectorStatus::empty;
        BlockHeader blockHeader;
        RecoverPhase recoverPhase = RecoverPhase::searchingStartOrEmpty;

        mutable infra::IntrusiveForwardList<Iterator> iterators;
    };
}

#endif
