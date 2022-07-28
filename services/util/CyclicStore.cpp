#include "infra/event/EventDispatcher.hpp"
#include "services/util/CyclicStore.hpp"

namespace services
{
    CyclicStore::CyclicStore(hal::Flash& flash)
        : flash(flash)
        , claimerAdd(resource)
        , claimerClear(resource)
        , claimerRecover(resource)
        , blockHeader()
    {
        Recover();
    }

    void CyclicStore::Add(infra::ConstByteRange range, const infra::Function<void()>& onDone)
    {
        assert(!range.empty());

        if (partialAddStarted)
            remainingPartialSize -= range.size();

        onAddDone = onDone;

        if (partialAddStarted)
            AddClaimed(range);
        else
            claimerAdd.Claim([this, range]() { AddClaimed(range); });
    }

    void CyclicStore::AddPartial(infra::ConstByteRange range, uint32_t totalSize, const infra::Function<void()>& onDone)
    {
        if (!partialAddStarted)
            remainingPartialSize = totalSize - range.size();

        Add(range, onDone);
    }

    void CyclicStore::AddClaimed(infra::ConstByteRange range)
    {
        assert(sequencer.Finished());
        sequencer.Load([this, range]()
        {
            sequencer.If([this]() { return !partialAddStarted; });
                FillSectorIfDataDoesNotFit(range.size() + remainingPartialSize);
                EraseSectorIfAtStart();
                WriteSectorStatusIfAtStartOfSector();
            sequencer.EndIf();
            WriteRange(range);
            sequencer.Execute([this]()
            {
                if (!partialAddStarted)
                    claimerAdd.Release();

                infra::EventDispatcher::Instance().Schedule([this]() { onAddDone(); });
            });
        });
    }

    void CyclicStore::Clear(const infra::Function<void()>& onDone)
    {
        onClearDone = onDone;
        claimerClear.Claim([this]() { ClearClaimed(); });
    }

    void CyclicStore::ClearUrgent(const infra::Function<void()>& onDone)
    {
        onClearDone = onDone;
        claimerClear.ClaimUrgent([this]() { ClearClaimed(); });
    }

    void CyclicStore::ClearClaimed()
    {
        endAddress = 0;
        startAddress = 0;

        assert(sequencer.Finished());
        sequencer.Load([this]()
        {
            sequencer.Step([this]()
            {
                flash.EraseAll([this]() { sequencer.Continue(); });
            });
            sequencer.Execute([this]()
            {
                claimerClear.Release();
                onClearDone();
            });
        });
    }

    CyclicStore::Iterator CyclicStore::Begin() const
    {
        return Iterator(*this);
    }

    void CyclicStore::Recover()
    {
        claimerRecover.Claim([this]()
        {
            endAddress = 0;
            startAddress = 0;

            assert(sequencer.Finished());
            sequencer.Load([this]() {
                sequencer.ForEach(sectorIndex, 0, flash.NumberOfSectors());
                    RecoverSector(sectorIndex);
                sequencer.EndForEach(sectorIndex);
                sequencer.If([this]() { return recoverPhase == RecoverPhase::corrupt || recoverPhase == RecoverPhase::searchingStartOrEmpty; });
                    sequencer.Step([this]()
                    {
                        startAddress = 0;
                        endAddress = startAddress;
                        flash.EraseAll([this]() { sequencer.Continue(); });
                    });
                sequencer.ElseIf([this]() { return endAddress != startAddress || recoverPhase != RecoverPhase::checkingAllEmpty; });
                    RecoverEndAddress();
                sequencer.EndIf();
                sequencer.Execute([this]()
                    {
                        if (flash.SectorOfAddress(endAddress) >= flash.SectorOfAddress(startAddress))
                            endSanitizeSector = flash.SectorOfAddress(endAddress);
                        else
                            endSanitizeSector = flash.SectorOfAddress(endAddress) + flash.NumberOfSectors();
                    });
                sequencer.ForEach(sectorIndex, flash.SectorOfAddress(startAddress), endSanitizeSector);
                    SanitizeSector(sectorIndex);
                sequencer.EndForEach(sectorIndex);
                UpdateStartAddressInLastSector();
                sequencer.Execute([this]()
                {
                    claimerRecover.Release();
                    recoverPhase = RecoverPhase::done;
                });
            });
        });
    }

    void CyclicStore::RecoverSector(uint32_t sectorIndex)
    {
        sequencer.Step([this, sectorIndex]()
        {
            flash.ReadBuffer(infra::MakeByteRange(sectorStatus), flash.AddressOfSector(sectorIndex), [this]() { sequencer.Continue(); });
        });
        sequencer.Execute([this, sectorIndex]()
        {
            if (sectorStatus == SectorStatus::firstInCycle)
            {
                if (recoverPhase == RecoverPhase::searchingStartOrEmpty)
                {
                    startAddress = flash.AddressOfSector(sectorIndex);
                    endAddress = startAddress;

                    if (sectorIndex == 0)
                        recoverPhase = RecoverPhase::checkingUsedFollowedByEmpty;
                    else
                        recoverPhase = RecoverPhase::checkingAllUsed;
                }
                else
                    recoverPhase = RecoverPhase::corrupt;
            }
            else if (sectorStatus == SectorStatus::used)
            {
                if (recoverPhase == RecoverPhase::checkingAllUsedOrAllEmpty)
                    recoverPhase = RecoverPhase::checkingAllUsed;
                else if (recoverPhase == RecoverPhase::checkingUsedFollowedByEmpty
                    || recoverPhase == RecoverPhase::searchingStartOrEmpty
                    || recoverPhase == RecoverPhase::checkingAllUsed)
                {}
                else
                    recoverPhase = RecoverPhase::corrupt;
            }
            else if (sectorStatus == SectorStatus::empty)
            {
                if (recoverPhase == RecoverPhase::searchingStartOrEmpty)
                {
                    startAddress = flash.StartOfNextSectorCyclical(flash.AddressOfSector(sectorIndex));
                    endAddress = flash.AddressOfSector(sectorIndex);

                    recoverPhase = RecoverPhase::checkingAllUsedOrAllEmpty;
                }
                else if (recoverPhase == RecoverPhase::checkingAllUsedOrAllEmpty)
                {
                    startAddress = 0;
                    endAddress = 0;
                    recoverPhase = RecoverPhase::checkingAllEmpty;
                }
                else if (recoverPhase == RecoverPhase::checkingUsedFollowedByEmpty)
                {
                    endAddress = flash.AddressOfSector(sectorIndex);
                    recoverPhase = RecoverPhase::checkingAllEmpty;
                }
                else if (recoverPhase == RecoverPhase::checkingAllEmpty)
                {}
                else
                    recoverPhase = RecoverPhase::corrupt;
            }
            else
                recoverPhase = RecoverPhase::corrupt;
        });
    }

    void CyclicStore::RecoverEndAddress()
    {
        sequencer.Step([this]()
        {
            endAddress = flash.StartOfPreviousSectorCyclical(endAddress);
            ++endAddress;

            flash.ReadBuffer(infra::MakeByteRange(blockHeader), endAddress, [this]() { sequencer.Continue(); });
        });
        sequencer.While([this]() { return blockHeader.status != BlockStatus::empty && !flash.AtStartOfSector(endAddress); });
            sequencer.If([this]() { return blockHeader.status == BlockStatus::dataReady || blockHeader.status == BlockStatus::erased; });
                sequencer.Execute([this]()
                {
                    endAddress += sizeof(BlockHeader);
                });
                sequencer.If([this]() { return endAddress + blockHeader.BlockLength() <= flash.TotalSize() && flash.SectorOfAddress(endAddress) == flash.SectorOfAddress(endAddress + blockHeader.BlockLength()); });
                    sequencer.Step([this]()
                    {
                        endAddress += blockHeader.BlockLength();
                        flash.ReadBuffer(infra::MakeByteRange(blockHeader), endAddress, [this]() { sequencer.Continue(); });
                    });
                sequencer.Else();
                    sequencer.Execute([this]()
                    {
                        endAddress = flash.StartOfNextSectorCyclical(endAddress);
                    });
                sequencer.EndIf();
            sequencer.ElseIf([this]() { return blockHeader.status == BlockStatus::writingLength; });
                sequencer.Step([this]()
                {
                    endAddress += sizeof(BlockHeader);
                    flash.ReadBuffer(infra::MakeByteRange(blockHeader), endAddress, [this]() { sequencer.Continue(); });
                });
            sequencer.ElseIf([this]() { return blockHeader.status == BlockStatus::writingData; });
                sequencer.Execute([this]()
                {
                    endAddress += sizeof(BlockHeader) + blockHeader.BlockLength();
                });
                sequencer.If([this]() { return flash.AddressOffsetInSector(endAddress) <= flash.SizeOfSector(flash.SectorOfAddress(endAddress)); });
                    sequencer.Step([this]()
                    {
                        flash.ReadBuffer(infra::MakeByteRange(blockHeader), endAddress, [this]() { sequencer.Continue(); });
                    });
                sequencer.Else();
                    sequencer.Execute([this]()
                    {
                        // The BlockLength() field is corrupt, skip to next sector
                        endAddress = flash.StartOfNextSectorCyclical(endAddress);
                    });
                sequencer.EndIf();
            sequencer.ElseIf([this]() { return blockHeader.status == BlockStatus::emptyUntilEnd; });
                sequencer.Execute([this]()
                {
                    endAddress = flash.StartOfNextSectorCyclical(endAddress);
                });
            sequencer.Else();
                sequencer.Execute([this]()
                {
                    endAddress = flash.StartOfNextSectorCyclical(endAddress);
                });
            sequencer.EndIf();
        sequencer.EndWhile();
    }

    void CyclicStore::SanitizeSector(uint32_t sectorIndex)
    {
        sequencer.Execute([this, &sectorIndex]()
            {
                sanitizeAddress = flash.AddressOfSector(sectorIndex % flash.NumberOfSectors());

                assert(flash.SectorOfAddress(sanitizeAddress) != flash.SectorOfAddress(endAddress));

                if (sanitizeAddress == startAddress)
                    startAddress += sizeof(SectorStatus);

                sanitizeAddress += sizeof(SectorStatus);
            });
        sequencer.While([this, &sectorIndex]() { return flash.SectorOfAddress(sanitizeAddress) == sectorIndex % flash.NumberOfSectors(); });
            sequencer.Step([this, &sectorIndex]()
            {
                flash.ReadBuffer(infra::MakeByteRange(blockHeader), sanitizeAddress, [this]() { sequencer.Continue(); });
            });
            sequencer.Step([this]()
            {
                bool continueSequencer = true;
                auto oldSanitizeAddress = sanitizeAddress;

                if (blockHeader.status == BlockStatus::dataReady || blockHeader.status == BlockStatus::writingData || blockHeader.status == BlockStatus::erased)
                    sanitizeAddress = (sanitizeAddress + sizeof(blockHeader) + blockHeader.BlockLength()) % flash.TotalSize();
                else if (blockHeader.status == BlockStatus::writingLength)
                    sanitizeAddress = (sanitizeAddress + sizeof(blockHeader)) % flash.TotalSize();
                else if (blockHeader.status == BlockStatus::empty)
                {
                    continueSequencer = false;
                    blockHeader.status = BlockStatus::emptyUntilEnd;
                    flash.WriteBuffer(infra::MakeByteRange(blockHeader.status), sanitizeAddress, [this]() { sequencer.Continue(); });
                    sanitizeAddress = flash.StartOfNextSectorCyclical(sanitizeAddress);
                }
                else if (blockHeader.status == BlockStatus::emptyUntilEnd)
                    sanitizeAddress = flash.StartOfNextSectorCyclical(sanitizeAddress);
                else
                {
                    sanitizeAddress = flash.StartOfNextSectorCyclical(sanitizeAddress);
                    oldSanitizeAddress = sanitizeAddress;
                }

                if (flash.AddressOffsetInSector(sanitizeAddress) >= flash.SizeOfSector(flash.SectorOfAddress(sanitizeAddress)) - sizeof(blockHeader))
                    sanitizeAddress = flash.StartOfNextSectorCyclical(sanitizeAddress);

                if (blockHeader.status != BlockStatus::dataReady && startAddress == oldSanitizeAddress)
                    startAddress = sanitizeAddress;

                assert(sanitizeAddress < flash.TotalSize() - sizeof(blockHeader));

                if (continueSequencer)
                    infra::EventDispatcher::Instance().Schedule([this]() { sequencer.Continue(); });
            });
        sequencer.EndWhile();
    }

    void CyclicStore::UpdateStartAddressInLastSector()
    {
        sequencer.Execute([this]()
            {
                sanitizeAddress = startAddress;
            });
        sequencer.While([this]() { return sanitizeAddress == startAddress && flash.SectorOfAddress(startAddress) == flash.SectorOfAddress(endAddress) && startAddress < endAddress; });
            sequencer.Step([this]()
                {
                    if (flash.AddressOffsetInSector(sanitizeAddress) == 0)
                        sanitizeAddress += sizeof(SectorStatus);

                    flash.ReadBuffer(infra::MakeByteRange(blockHeader), sanitizeAddress, [this]() { sequencer.Continue(); });
                });
            sequencer.Execute([this]()
                {
                    if (blockHeader.status == BlockStatus::writingData || blockHeader.status == BlockStatus::erased)
                    {
                        sanitizeAddress = (sanitizeAddress + sizeof(blockHeader) + blockHeader.BlockLength()) % flash.TotalSize();
                        startAddress = sanitizeAddress;
                    }
                    else if (blockHeader.status == BlockStatus::writingLength)
                    {
                        sanitizeAddress = (sanitizeAddress + sizeof(blockHeader)) % flash.TotalSize();
                        startAddress = sanitizeAddress;
                    }
                    else
                        sanitizeAddress = flash.StartOfNextSectorCyclical(sanitizeAddress);
                });
        sequencer.EndWhile();
    }

    void CyclicStore::EraseSectorIfAtStart()
    {
        sequencer.If([this]() { return flash.AddressOffsetInSector(endAddress) == 0; });
            sequencer.Step([this]()
            {
                flash.EraseSector(flash.SectorOfAddress(endAddress), [this]() { sequencer.Continue(); });
            });
            sequencer.Step([this]()
            {
                flash.ReadBuffer(infra::MakeByteRange(sectorStatus), flash.StartOfNextSectorCyclical(endAddress), [this]() { sequencer.Continue(); });
            });
            sequencer.If([this]() { return sectorStatus == SectorStatus::used && flash.SectorOfAddress(startAddress) == flash.SectorOfAddress(endAddress); });
                sequencer.Step([this]()
                {
                    for (auto& iterator: iterators)
                        iterator.SectorIsErased(flash.SectorOfAddress(startAddress));

                    sectorStatus = SectorStatus::firstInCycle;
                    startAddress = flash.StartOfNextSectorCyclical(endAddress);
                    flash.WriteBuffer(infra::MakeByteRange(sectorStatus), startAddress, [this]() { sequencer.Continue(); });
                });
            sequencer.EndIf();
        sequencer.EndIf();
    }

    void CyclicStore::FillSectorIfDataDoesNotFit(std::size_t size)
    {
        sequencer.If([this, size]() { return flash.AddressOffsetInSector(endAddress) + sizeof(BlockHeader) + size > flash.SizeOfSector(flash.SectorOfAddress(endAddress)); });
            sequencer.Step([this]()
            {
                blockHeader.status = BlockStatus::emptyUntilEnd;
                flash.WriteBuffer(infra::MakeByteRange(blockHeader.status), endAddress, [this]() { sequencer.Continue(); });
                endAddress = flash.StartOfNextSectorCyclical(endAddress);
            });
        sequencer.EndIf();
    }

    void CyclicStore::WriteSectorStatusIfAtStartOfSector()
    {
        sequencer.If([this]() { return flash.AtStartOfSector(endAddress); });
            sequencer.Step([this]()
            {
                flash.ReadBuffer(infra::MakeByteRange(sectorStatus), flash.StartOfNextSectorCyclical(endAddress), [this]() { sequencer.Continue(); });
            });
            sequencer.If([this]() { return sectorStatus == SectorStatus::used; });
                sequencer.Step([this]()
                {
                    sectorStatus = SectorStatus::firstInCycle;
                    flash.WriteBuffer(infra::MakeByteRange(sectorStatus), flash.StartOfNextSectorCyclical(endAddress), [this]() { sequencer.Continue(); });
                });
            sequencer.EndIf(),
            sequencer.Step([this]()
            {
                sectorStatus = endAddress == startAddress ? SectorStatus::firstInCycle : SectorStatus::used;
                flash.WriteBuffer(infra::MakeByteRange(sectorStatus), endAddress, [this]() { sequencer.Continue(); });
                ++endAddress;
                if (endAddress == flash.TotalSize())
                    endAddress = 0;
            });
        sequencer.EndIf();
    }

    void CyclicStore::WriteRange(infra::ConstByteRange range)
    {
        sequencer.If([this]() { return !partialAddStarted; });
            sequencer.Step([this, range]()  // Write status 'writing length'
            {
                assert(sizeof(BlockHeader) + range.size() + remainingPartialSize + 1 <= flash.SizeOfSector(flash.SectorOfAddress(endAddress)));
                blockHeader.status = BlockStatus::writingLength;
                flash.WriteBuffer(infra::MakeByteRange(blockHeader.status), endAddress, [this]() { sequencer.Continue(); });
            });
            sequencer.Step([this, range]()  // Write length
            {
                blockHeader.SetBlockLength(static_cast<Length>(range.size() + remainingPartialSize));
                flash.WriteBuffer(infra::ByteRange(&blockHeader.lengthLsb, (&blockHeader.lengthMsb)+1), endAddress + 1, [this]() { sequencer.Continue(); });
            });
            sequencer.Step([this]()         // Write status 'writing data'
            {
                blockHeader.status = BlockStatus::writingData;
                flash.WriteBuffer(infra::MakeByteRange(blockHeader.status), endAddress + partialSizeWritten, [this]() { sequencer.Continue(); });
            });
        sequencer.EndIf();
        sequencer.Step([this, range]()  // Write data
        {
            flash.WriteBuffer(range, endAddress + partialSizeWritten + 3, [this]() { sequencer.Continue(); });
            partialSizeWritten += range.size();
        });
        sequencer.Step([this, range]()  // Write status 'ready'
        {
            if (remainingPartialSize == 0)
            {
                blockHeader.status = BlockStatus::dataReady;
                flash.WriteBuffer(infra::MakeByteRange(blockHeader.status), endAddress, [this]() { sequencer.Continue(); });
                endAddress += partialSizeWritten + 3;
                partialSizeWritten = 0;
                partialAddStarted = false;
                if (endAddress == flash.TotalSize())
                    endAddress = 0;
            }
            else
            {
                partialAddStarted = true;
                infra::EventDispatcher::Instance().Schedule([this]() { sequencer.Continue(); });
            }
        });
    }

    CyclicStore::Iterator::Iterator(const CyclicStore& store)
        : store(store)
        , loadStartAddressDelayed(true)
        , address(0)
        , claimer(store.resource)
    {
        store.iterators.push_front(*this);
    }

    CyclicStore::Iterator::Iterator(const Iterator& other)
        : store(other.store)
        , loadStartAddressDelayed(other.loadStartAddressDelayed)
        , address(other.address)
        , claimer(store.resource)
    {
        store.iterators.push_front(*this);
    }

    CyclicStore::Iterator::~Iterator()
    {
        store.iterators.erase_slow(*this);
    }

    CyclicStore::Iterator& CyclicStore::Iterator::operator=(const Iterator& other)
    {
        assert(&store == &other.store);

        loadStartAddressDelayed = other.loadStartAddressDelayed;
        address = other.address;

        sectorStatus = other.sectorStatus;
        firstSectorToRead = other.firstSectorToRead;
        blockHeader = other.blockHeader;
        readBuffer = other.readBuffer;
        found = other.found;

        return *this;
    }

    void CyclicStore::Iterator::Read(infra::ByteRange buffer, const infra::Function<void(infra::ByteRange result)>& onDone)
    {
        readBuffer = buffer;
        found = false;
        reachedEnd = false;

        claimer.Claim([this, onDone]()
        {
            if (loadStartAddressDelayed)
            {
                loadStartAddressDelayed = false;
                address = store.startAddress;
            }

            assert(sequencer.Finished());
            sequencer.Load([this, onDone]()
            {
                ReadSectorStatusIfAtStart();
                sequencer.While([this]() { return sectorStatus == SectorStatus::used && !found && !reachedEnd; });
                    ReadBlockHeader();
                    sequencer.If([this]() { return blockHeader.status == BlockStatus::dataReady; });
                        sequencer.If([this]() { return store.flash.AddressOffsetInSector(address) + blockHeader.BlockLength() <= store.flash.SizeOfSector(store.flash.SectorOfAddress(address)); });
                            ReadData();
                        sequencer.Else();
                            sequencer.Execute([this]()
                            {
                                UpdateAddress(store.flash.StartOfNextSectorCyclical(address));
                            });
                        sequencer.EndIf();
                    IncreaseAddressForNonData();
                    sequencer.EndIf();
                    ReadSectorStatusIfAtStart();
                sequencer.EndWhile();
                sequencer.Execute([this, onDone]()
                {
                    if (!found)
                        readBuffer.clear();

                    auto onDoneCopy = onDone;
                    auto readBufferCopy = readBuffer;
                    claimer.Release();
                    onDoneCopy(readBufferCopy);
                });
            });
        });
    }

    void CyclicStore::Iterator::ErasePrevious(const infra::Function<void()>& onDone)
    {
        claimer.Claim([this, onDone]()
        {
            assert(sequencer.Finished());
            sequencer.Load([this, onDone]()
            {
                sequencer.If([this]() {  return !previousErased; });
                    ErasePreviousData();
                sequencer.EndIf();
                sequencer.Execute([this, onDone]()
                {
                    auto onDoneCopy = onDone;
                    claimer.Release();
                    onDoneCopy();
                });
            });
        });
    }

    void CyclicStore::Iterator::SectorIsErased(uint32_t sectorIndex)
    {
        if (store.flash.SectorOfAddress(address) == sectorIndex)
        {
            UpdateAddress(store.flash.StartOfNextSectorCyclical(address));
            firstSectorToRead = true;
        }

        if (!previousErased && store.flash.SectorOfAddress(addressPreviousBlockHeader) == sectorIndex)
            previousErased = true;
    }

    bool CyclicStore::Iterator::operator==(const Iterator& other) const
    {
        return (loadStartAddressDelayed ? store.startAddress : address) == (other.loadStartAddressDelayed ? other.store.startAddress : other.address);
    }

    void CyclicStore::Iterator::ReadSectorStatusIfAtStart()
    {
        sequencer.Execute([this]()
            {
                if (store.flash.SectorOfAddress(address) != store.flash.SectorOfAddress(address + sizeof(blockHeader) - 1))
                    UpdateAddress(store.flash.StartOfNextSector(address));
            });
        sequencer.If([this]() { return store.flash.AtStartOfSector(address); });
            sequencer.Step([this]()
            {
                store.flash.ReadBuffer(infra::MakeByteRange(sectorStatus), address, [this]() { sequencer.Continue(); });
            });
            sequencer.Execute([this]()
            {
                if (sectorStatus != SectorStatus::empty)
                    UpdateAddress(address + sizeof(sectorStatus));

                if (sectorStatus == SectorStatus::firstInCycle)
                {
                    if (firstSectorToRead)
                        sectorStatus = SectorStatus::used;
                    else
                        sectorStatus = SectorStatus::empty;
                }

                firstSectorToRead = false;
            });
        sequencer.EndIf();
    }

    void CyclicStore::Iterator::ReadBlockHeader()
    {
        sequencer.Step([this]()
        {
            store.flash.ReadBuffer(infra::MakeByteRange(blockHeader), address, [this]() { sequencer.Continue(); });
        });
        sequencer.Execute([this]()
        {
            uint32_t newAddress;
            if (store.flash.SectorOfAddress(address + sizeof(blockHeader)) == store.flash.SectorOfAddress(address))
                newAddress = address + sizeof(blockHeader);
            else
                newAddress = store.flash.StartOfNextSectorCyclical(address);

            if (blockHeader.status == BlockStatus::dataReady)
                address = newAddress;
            else
                UpdateAddress(newAddress);
        });
    }

    void CyclicStore::Iterator::ReadData()
    {
        sequencer.Step([this]()
        {
            readBuffer.shrink_from_back_to(blockHeader.BlockLength());
            store.flash.ReadBuffer(readBuffer, address, [this]() { sequencer.Continue(); });
            addressPreviousBlockHeader = address - sizeof(blockHeader);
            previousErased = false;
            address = (address + blockHeader.BlockLength()) % store.flash.TotalSize();
            found = true;
        });
    }

    void CyclicStore::Iterator::ErasePreviousData()
    {
        sequencer.Step([this]()
        {
            blockHeader.status = BlockStatus::erased;
            previousErased = true;
            store.flash.WriteBuffer(infra::MakeByteRange(blockHeader.status), addressPreviousBlockHeader, [this]() { sequencer.Continue(); });

            if (addressPreviousBlockHeader == store.startAddress || addressPreviousBlockHeader == store.startAddress + 1)   // For start of the sector, the block header starts 1 byte beyond startAddress
                store.startAddress = address;
        });
    }

    void CyclicStore::Iterator::IncreaseAddressForNonData()
    {
        sequencer.ElseIf([this]() { return blockHeader.status == BlockStatus::emptyUntilEnd; });
            sequencer.Execute([this]()
            {
                if (!store.flash.AtStartOfSector(address))
                    UpdateAddress(store.flash.StartOfNextSectorCyclical(address));
            });
        sequencer.ElseIf([this]() { return blockHeader.status == BlockStatus::empty; });
            sequencer.Execute([this]()
            {
                reachedEnd = true;
                UpdateAddress(address - sizeof(blockHeader));
            });
        sequencer.ElseIf([this]() { return blockHeader.status == BlockStatus::writingLength; });
            // Do nothing
        sequencer.ElseIf([this]() { return blockHeader.status == BlockStatus::writingData || blockHeader.status == BlockStatus::erased; });
            sequencer.Execute([this]()
            {
                UpdateAddress(address + blockHeader.BlockLength());
            });
        sequencer.Else();
            sequencer.Execute([this]()
            {
                UpdateAddress(store.flash.StartOfNextSectorCyclical(address));
            });
    }

    void CyclicStore::Iterator::UpdateAddress(uint32_t newAddress)
    {
        if (store.startAddress == address)
            store.startAddress = newAddress;

        address = newAddress;
    }
}
