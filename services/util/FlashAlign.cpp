#include "services/util/FlashAlign.hpp"

namespace services
{
    FlashAlign::FlashAlign(infra::ByteRange firstChunk, infra::ByteRange lastChunk)
        : firstChunk(firstChunk)
        , lastChunk(lastChunk)
        , align(static_cast<uint8_t>(firstChunk.size()))
    {}

    void FlashAlign::Align(uint32_t address, infra::ConstByteRange buffer)
    {
        chunks.clear();
        alignedAddress = 0;

        if (buffer.empty())
            return;

        uint32_t newAddress = AlignFirst(address, buffer);
        newAddress = AlignFullWords(newAddress, buffer);
        AlignLast(newAddress, buffer);
    }

    uint32_t FlashAlign::AlignFirst(uint32_t address, infra::ConstByteRange& buffer)
    {
        uint32_t nrPrependBytes = address % align;
        alignedAddress = address - nrPrependBytes;

        if (nrPrependBytes)
        {
            std::fill(firstChunk.begin(), firstChunk.end(), 0xff);

            uint8_t bytesToAdd = std::min<uint32_t>(align - nrPrependBytes, buffer.size());
            std::copy(reinterpret_cast<const uint8_t*>(&buffer.front()), reinterpret_cast<const uint8_t*>(&buffer.front()) + bytesToAdd, firstChunk.begin() + nrPrependBytes);

            buffer.pop_front(bytesToAdd);

            Chunk chunk(alignedAddress, firstChunk);
            chunks.push_back(chunk);

            return chunk.alignedAddress + align;
        }

        return alignedAddress;
    }

    uint32_t FlashAlign::AlignFullWords(uint32_t address, infra::ConstByteRange& buffer)
    {
        uint32_t remainingAlignedWords = buffer.size() / align;

        if (remainingAlignedWords)
        {
            uint32_t remainingBytes = buffer.size() - (remainingAlignedWords * align);

            Chunk chunk(address, infra::ConstByteRange(buffer.begin(), buffer.begin() + remainingAlignedWords*align));
            chunks.push_back(chunk);

            address += chunk.data.size();
            buffer.pop_front(chunk.data.size());
        }

        return address;
    }

    void FlashAlign::AlignLast(uint32_t address, infra::ConstByteRange& buffer)
    {
        if (!buffer.empty())
        {
            std::fill(lastChunk.begin(), lastChunk.end(), 0xff);
            std::copy(reinterpret_cast<const uint8_t*>(&buffer.front()), reinterpret_cast<const uint8_t*>(&buffer.front()) + buffer.size(), lastChunk.begin());

            Chunk chunk(address, lastChunk);
            chunks.push_back(chunk);
        }
    }

    FlashAlign::Chunk* FlashAlign::First()
    {
        if (chunks.empty())
            return nullptr;

        return &chunks.front();
    }

    FlashAlign::Chunk* FlashAlign::Next()
    {
        if (!chunks.empty())
            chunks.pop_front();

        return First();
    }
}