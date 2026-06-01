#include "services/util/FlashEcho.hpp"
#include "infra/util/ReallyAssert.hpp"
#include <algorithm>

namespace services
{
    FlashEcho::FlashEcho(services::Echo& echo, hal::Flash& flash)
        : flash::Flash(echo)
        , flashResult(echo)
        , flash(flash)
    {}

    void FlashEcho::Stop(const infra::Function<void()>& onDone)
    {
        onStopped = onDone;

        if (!busy)
            onStopped();
    }

    void FlashEcho::Read(uint32_t address, uint32_t size)
    {
        really_assert(!busy);
        busy = true;

        flash.ReadBuffer(infra::Head(infra::MakeRange(buffer), size), address, [this, size]()
            {
                busy = false;

                if (onStopped)
                    onStopped();
                else
                    flashResult.RequestSend([this, size]()
                        {
                            flashResult.ReadDone(infra::Head(infra::MakeRange(buffer), size));
                            MethodDone();

                            if (onStopped)
                                onStopped();
                        });
            });
    }

    void FlashEcho::Write(uint32_t address, infra::ConstByteRange contents)
    {
        really_assert(!busy);
        busy = true;

        flash.WriteBuffer(contents, address, [this]()
            {
                busy = false;

                if (onStopped)
                    onStopped();
                else
                    flashResult.RequestSend([this]()
                        {
                            flashResult.WriteDone();
                            MethodDone();

                            if (onStopped)
                                onStopped();
                        });
            });
    }

    void FlashEcho::EraseSectors(uint32_t sector, uint32_t numberOfSectors)
    {
        really_assert(!busy);
        busy = true;

        flash.EraseSectors(sector, sector + numberOfSectors, [this]()
            {
                busy = false;
                if (onStopped)
                    onStopped();
                else
                    flashResult.RequestSend([this]()
                        {
                            flashResult.EraseSectorsDone();
                            MethodDone();

                            if (onStopped)
                                onStopped();
                        });
            });
    }

    FlashEchoProxyBase::FlashEchoProxyBase(services::Echo& echo, infra::MemoryRange<const uint32_t> sectorSizes)
        : flash::FlashResult(echo)
        , sectorSizes(sectorSizes)
        , proxy(echo)
    {}

    FlashEchoProxy::FlashEchoProxy(services::Echo& echo, infra::MemoryRange<const uint32_t> sectorSizes)
        : FlashEchoProxyBase(echo, sectorSizes)
    {}

    FlashEchoSequentialProxy::FlashEchoSequentialProxy(services::Echo& echo, infra::MemoryRange<const uint32_t> sectorSizes)
        : FlashEchoProxyBase(echo, sectorSizes)
    {}

    uint32_t FlashEchoProxyBase::NumberOfSectors() const
    {
        return sectorSizes.size();
    }

    uint32_t FlashEchoProxyBase::SizeOfSector(uint32_t sectorIndex) const
    {
        return sectorSizes[sectorIndex];
    }

    uint32_t FlashEchoProxyBase::SectorOfAddress(uint32_t address) const
    {
        uint32_t totalSize = 0;
        for (uint32_t sector = 0; sector != sectorSizes.size(); ++sector)
        {
            totalSize += sectorSizes[sector];
            if (address < totalSize)
                return sector;
        }

        assert(address == totalSize);
        return sectorSizes.size();
    }

    uint32_t FlashEchoProxyBase::AddressOfSector(uint32_t sectorIndex) const
    {
        uint32_t address = 0;
        for (uint32_t sector = 0; sector != sectorIndex; ++sector)
            address += sectorSizes[sector];
        return address;
    }

    void FlashEchoProxyBase::WriteBuffer(infra::ConstByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        really_assert(!this->onDone);

        bufferPosition = 0;
        readingBuffer = {};
        writingBuffer = buffer;
        this->onDone = onDone;
        this->address = address;

        WritePartialBuffer(address, 0);
    }

    void FlashEchoProxyBase::ReadBuffer(infra::ByteRange buffer, uint32_t address, infra::Function<void()> onDone)
    {
        really_assert(!this->onDone);

        bufferPosition = 0;
        this->address = address;
        writingBuffer = {};
        readingBuffer = buffer;
        this->onDone = onDone;

        ReadPartialBuffer(address, 0);
    }

    void FlashEchoProxyBase::EraseSectors(uint32_t beginIndex, uint32_t endIndex, infra::Function<void()> onDone)
    {
        really_assert(!this->onDone);

        readingBuffer = {};
        writingBuffer = {};
        this->onDone = onDone;
        this->endIndex = endIndex;

        proxy.RequestSend([this, beginIndex]()
            {
                proxy.EraseSectors(beginIndex, this->endIndex - beginIndex);
            });
    }

    void FlashEchoProxyBase::ReadDone(infra::ConstByteRange contents)
    {
        really_assert(onDone != nullptr);
        really_assert(readingBuffer.size() > bufferPosition);
        really_assert(contents.size() <= readingBuffer.size() - bufferPosition);
        really_assert(writingBuffer.empty());

        infra::Copy(contents, infra::Head(infra::DiscardHead(readingBuffer, bufferPosition), contents.size()));
        bufferPosition += contents.size();

        if (bufferPosition == readingBuffer.size())
            onDone();
        else
            OnReadIncomplete(address, bufferPosition);

        MethodDone();
    }

    void FlashEchoProxyBase::WriteDone()
    {
        really_assert(onDone != nullptr);
        really_assert(bufferPosition < writingBuffer.size());
        really_assert(readingBuffer.empty());

        bufferPosition += std::min<std::size_t>(writingBuffer.size() - bufferPosition, flash::WriteRequest::contentsSize);

        if (bufferPosition == writingBuffer.size())
            onDone();
        else
            OnWriteIncomplete(address, bufferPosition);

        MethodDone();
    }

    void FlashEchoProxyBase::EraseSectorsDone()
    {
        really_assert(onDone != nullptr);
        really_assert(readingBuffer.empty());
        really_assert(writingBuffer.empty());

        onDone();
        MethodDone();
    }

    void FlashEchoProxyBase::OnReadIncomplete([[maybe_unused]] uint32_t address, [[maybe_unused]] uint32_t bufferPosition)
    {}

    void FlashEchoProxyBase::OnWriteIncomplete([[maybe_unused]] uint32_t address, [[maybe_unused]] uint32_t bufferPosition)
    {}

    void FlashEchoProxyBase::OnReadChunkSent([[maybe_unused]] uint32_t address, [[maybe_unused]] uint32_t nextStart)
    {}

    void FlashEchoProxyBase::OnWriteChunkSent([[maybe_unused]] uint32_t address, [[maybe_unused]] uint32_t nextStart)
    {}

    void FlashEchoProxyBase::ReadPartialBuffer(uint32_t address, uint32_t start)
    {
        if (readingBuffer.size() > start)
        {
            this->start = start;
            proxy.RequestSend([this, address]()
                {
                    auto size = std::min<uint32_t>(readingBuffer.size() - this->start, flash::WriteRequest::contentsSize);
                    proxy.Read(address + this->start, size);
                    OnReadChunkSent(address, this->start + size);
                });
        }
    }

    void FlashEchoProxyBase::WritePartialBuffer(uint32_t address, uint32_t start)
    {
        if (writingBuffer.size() > start)
        {
            this->start = start;
            proxy.RequestSend([this, address]()
                {
                    auto size = std::min<uint32_t>(writingBuffer.size() - this->start, flash::WriteRequest::contentsSize);
                    proxy.Write(address + this->start, infra::Head(infra::DiscardHead(writingBuffer, this->start), size));
                    OnWriteChunkSent(address, this->start + size);
                });
        }
    }

    void FlashEchoProxy::OnReadChunkSent(uint32_t address, uint32_t nextStart)
    {
        ReadPartialBuffer(address, nextStart);
    }

    void FlashEchoProxy::OnWriteChunkSent(uint32_t address, uint32_t nextStart)
    {
        WritePartialBuffer(address, nextStart);
    }

    void FlashEchoSequentialProxy::OnReadIncomplete(uint32_t address, uint32_t nextStart)
    {
        ReadPartialBuffer(address, nextStart);
    }

    void FlashEchoSequentialProxy::OnWriteIncomplete(uint32_t address, uint32_t nextStart)
    {
        WritePartialBuffer(address, nextStart);
    }

    template<class T>
    FlashEchoHomogeneousProxyBase<T>::FlashEchoHomogeneousProxyBase(services::Echo& echo, uint32_t numberOfSectors, uint32_t sizeOfEachSector)
        : T(echo, infra::MemoryRange<const uint32_t>{})
        , numberOfSectors(numberOfSectors)
        , sizeOfEachSector(sizeOfEachSector)
    {}

    template<class T>
    uint32_t FlashEchoHomogeneousProxyBase<T>::NumberOfSectors() const
    {
        return numberOfSectors;
    }

    template<class T>
    uint32_t FlashEchoHomogeneousProxyBase<T>::SizeOfSector(uint32_t sectorIndex) const
    {
        return sizeOfEachSector;
    }

    template<class T>
    uint32_t FlashEchoHomogeneousProxyBase<T>::SectorOfAddress(uint32_t address) const
    {
        return address / sizeOfEachSector;
    }

    template<class T>
    uint32_t FlashEchoHomogeneousProxyBase<T>::AddressOfSector(uint32_t sectorIndex) const
    {
        return sectorIndex * sizeOfEachSector;
    }

    template class FlashEchoHomogeneousProxyBase<FlashEchoProxy>;
    template class FlashEchoHomogeneousProxyBase<FlashEchoSequentialProxy>;
}
