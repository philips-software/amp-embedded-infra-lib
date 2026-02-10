#include "services/synchronous_util/SynchronousFlashAligner.hpp"
#include "infra/util/ReallyAssert.hpp"
#include <limits>

namespace services
{
    SynchronousFlashAligner::SynchronousFlashAligner(infra::BoundedVector<uint8_t>& buffer, hal::SynchronousFlash& delegate)
        : SynchronousFlashDelegateBase(delegate)
        , alignedBuffer_(buffer)
    {}

    void SynchronousFlashAligner::Flush()
    {
        if (alignedBuffer_.empty())
        {
            return;
        }

        auto size = alignedBuffer_.size();

        really_assert(size <= std::numeric_limits<uint32_t>::max());
        really_assert(static_cast<uint32_t>(size) <= *address_);

        auto writeAddress = *address_ - static_cast<uint32_t>(size);
        really_assert(writeAddress <= TotalSize());
        really_assert(alignedBuffer_.max_size() <= TotalSize() - writeAddress);

        alignedBuffer_.resize(alignedBuffer_.max_size(), 0x00);
        SynchronousFlashDelegateBase::WriteBuffer(infra::MakeRange(alignedBuffer_), writeAddress);

        alignedBuffer_.clear();
        address_.reset();
    }

    void SynchronousFlashAligner::WriteBuffer(infra::ConstByteRange buffer, uint32_t address)
    {
        if (!alignedBuffer_.empty())
        {
            really_assert(!address_ || address == *address_);

            auto range = infra::Head(buffer, alignedBuffer_.max_size() - alignedBuffer_.size());

            really_assert(range.size() <= std::numeric_limits<uint32_t>::max());
            really_assert(address <= std::numeric_limits<uint32_t>::max() - static_cast<uint32_t>(range.size()));

            alignedBuffer_.insert(alignedBuffer_.end(), range.begin(), range.end());
            buffer.pop_front(range.size());
            address += static_cast<uint32_t>(range.size());
        }

        if (alignedBuffer_.full())
        {
            auto remainingBuffer = buffer;
            auto remainingAddress = address;

            really_assert(alignedBuffer_.size() <= std::numeric_limits<uint32_t>::max());
            really_assert(static_cast<uint32_t>(alignedBuffer_.size()) <= remainingAddress);

            auto writeAddress = remainingAddress - static_cast<uint32_t>(alignedBuffer_.size());

            really_assert(writeAddress <= TotalSize());
            really_assert(alignedBuffer_.size() <= TotalSize() - writeAddress);

            SynchronousFlashDelegateBase::WriteBuffer(infra::MakeRange(alignedBuffer_), writeAddress);
            alignedBuffer_.clear();
            WriteBuffer(remainingBuffer, remainingAddress);
        }
        else
        {
            auto sendNowSize = buffer.size() - buffer.size() % alignedBuffer_.max_size();
            auto sendNow = infra::Head(buffer, sendNowSize);
            auto sendLater = infra::DiscardHead(buffer, sendNowSize);

            if (!address_)
            {
                really_assert(address % alignedBuffer_.max_size() == 0);
            }

            alignedBuffer_.insert(alignedBuffer_.end(), sendLater.begin(), sendLater.end());

            really_assert(buffer.size() <= std::numeric_limits<uint32_t>::max());
            really_assert(address <= std::numeric_limits<uint32_t>::max() - static_cast<uint32_t>(buffer.size()));

            address_ = address + static_cast<uint32_t>(buffer.size());

            if (sendNow.empty())
            {
                return;
            }

            really_assert(address <= TotalSize());
            really_assert(sendNow.size() <= TotalSize() - address);

            SynchronousFlashDelegateBase::WriteBuffer(sendNow, address);
        }
    }

    void SynchronousFlashAligner::ReadBuffer(infra::ByteRange buffer, uint32_t address)
    {
        really_assert(buffer.size() <= std::numeric_limits<uint32_t>::max());
        really_assert(!OverlapsWithBufferedData(address, address + static_cast<uint32_t>(buffer.size())));

        SynchronousFlashDelegateBase::ReadBuffer(buffer, address);
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
        if (alignedBuffer_.empty())
            return false;

        really_assert(alignedBuffer_.size() <= std::numeric_limits<uint32_t>::max());
        really_assert(static_cast<uint32_t>(alignedBuffer_.size()) <= *address_);

        auto bufferedStart = *address_ - static_cast<uint32_t>(alignedBuffer_.size());
        auto bufferedEnd = *address_;

        // Check if ranges overlap: ranges overlap if start of one is before end of the other, and vice versa
        return rangeStart < bufferedEnd && rangeEnd > bufferedStart;
    }
}
