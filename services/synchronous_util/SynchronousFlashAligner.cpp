#include "services/synchronous_util/SynchronousFlashAligner.hpp"
#include "infra/util/ReallyAssert.hpp"
#include <limits>

namespace services
{
    SynchronousFlashAligner::SynchronousFlashAligner(infra::BoundedVector<uint8_t>& buffer, hal::SynchronousFlash& delegate)
        : SynchronousFlashDelegateBase(delegate)
        , alignedBuffer(buffer)
    {}

    void SynchronousFlashAligner::Flush()
    {
        if (alignedBuffer.empty())
            return;

        auto writeAddress = *address - static_cast<uint32_t>(alignedBuffer.size());

        alignedBuffer.resize(alignedBuffer.max_size(), 0x00);
        SynchronousFlashDelegateBase::WriteBuffer(infra::MakeRange(alignedBuffer), writeAddress);

        alignedBuffer.clear();
        address = std::nullopt;
    }

    void SynchronousFlashAligner::FillAlignedBuffer(infra::ConstByteRange& buffer, uint32_t& destAddress)
    {
        really_assert(!address || destAddress == *address);

        auto range = infra::Head(buffer, alignedBuffer.max_size() - alignedBuffer.size());

        really_assert(destAddress <= std::numeric_limits<uint32_t>::max() - static_cast<uint32_t>(range.size()));

        alignedBuffer.insert(alignedBuffer.end(), range.begin(), range.end());
        buffer.pop_front(range.size());
        destAddress += static_cast<uint32_t>(range.size());
    }

    void SynchronousFlashAligner::FlushAlignedBuffer(uint32_t destAddress)
    {
        really_assert(alignedBuffer.size() <= std::numeric_limits<uint32_t>::max());
        really_assert(static_cast<uint32_t>(alignedBuffer.size()) <= destAddress);

        auto writeAddress = destAddress - static_cast<uint32_t>(alignedBuffer.size());

        really_assert(writeAddress <= TotalSize());
        really_assert(alignedBuffer.size() <= TotalSize() - writeAddress);

        SynchronousFlashDelegateBase::WriteBuffer(infra::MakeRange(alignedBuffer), writeAddress);
        alignedBuffer.clear();
    }

    void SynchronousFlashAligner::WriteAlignedData(infra::ConstByteRange buffer, uint32_t destAddress)
    {
        auto sendNowSize = buffer.size() - buffer.size() % alignedBuffer.max_size();
        auto sendNow = infra::Head(buffer, sendNowSize);
        auto sendLater = infra::DiscardHead(buffer, sendNowSize);

        really_assert(address || destAddress % alignedBuffer.max_size() == 0);
        really_assert(buffer.size() <= std::numeric_limits<uint32_t>::max());
        really_assert(destAddress <= std::numeric_limits<uint32_t>::max() - static_cast<uint32_t>(buffer.size()));
        really_assert(destAddress <= TotalSize());

        address = destAddress + static_cast<uint32_t>(buffer.size());

        alignedBuffer.insert(alignedBuffer.end(), sendLater.begin(), sendLater.end());

        really_assert(alignedBuffer.size() <= std::numeric_limits<uint32_t>::max());
        really_assert(static_cast<uint32_t>(alignedBuffer.size()) <= *address);
        auto bufferedStart = *address - static_cast<uint32_t>(alignedBuffer.size());
        really_assert(alignedBuffer.max_size() <= TotalSize() - bufferedStart);

        if (!sendNow.empty())
        {
            really_assert(sendNow.size() <= TotalSize() - destAddress);
            SynchronousFlashDelegateBase::WriteBuffer(sendNow, destAddress);
        }
    }

    void SynchronousFlashAligner::WriteBuffer(infra::ConstByteRange buffer, uint32_t destAddress)
    {
        while (!buffer.empty())
        {
            if (!alignedBuffer.empty())
                FillAlignedBuffer(buffer, destAddress);

            if (alignedBuffer.full())
            {
                FlushAlignedBuffer(destAddress);
            }
            else
            {
                WriteAlignedData(buffer, destAddress);
                break;
            }
        }
    }

    void SynchronousFlashAligner::ReadBuffer(infra::ByteRange buffer, uint32_t destAddress)
    {
        really_assert(buffer.size() <= std::numeric_limits<uint32_t>::max());
        really_assert(!OverlapsWithBufferedData(destAddress, destAddress + static_cast<uint32_t>(buffer.size())));

        SynchronousFlashDelegateBase::ReadBuffer(buffer, destAddress);
    }

    void SynchronousFlashAligner::EraseSectors(uint32_t beginIndex, uint32_t endIndex)
    {
        really_assert(beginIndex <= endIndex);

        if (beginIndex < endIndex)
        {
            auto eraseStart = AddressOfSector(beginIndex);
            auto eraseEnd = AddressOfSector(endIndex - 1);

            really_assert(eraseEnd <= TotalSize());
            really_assert(SizeOfSector(endIndex - 1) <= TotalSize() - eraseEnd);

            eraseEnd += SizeOfSector(endIndex - 1);

            really_assert(!OverlapsWithBufferedData(eraseStart, eraseEnd));
        }

        SynchronousFlashDelegateBase::EraseSectors(beginIndex, endIndex);
    }

    bool SynchronousFlashAligner::OverlapsWithBufferedData(uint32_t rangeStart, uint32_t rangeEnd) const
    {
        if (alignedBuffer.empty())
            return false;

        auto bufferedStart = *address - static_cast<uint32_t>(alignedBuffer.size());
        auto bufferedEnd = *address;

        return rangeStart < bufferedEnd && rangeEnd > bufferedStart;
    }
}
