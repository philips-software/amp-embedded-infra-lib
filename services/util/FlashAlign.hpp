#ifndef SERVICES_FLASH_ALIGN_HPP
#define SERVICES_FLASH_ALIGN_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/BoundedDeque.hpp"

namespace services
{
    class FlashAlign
    {
    public:

        template<std::size_t Size>
            using WithAlignment = infra::WithStorage<infra::WithStorage<FlashAlign, std::array<uint8_t, Size>>, std::array<uint8_t, Size>>;

        FlashAlign(infra::ByteRange firstChunk, infra::ByteRange lastChunk);
        void Align(uint32_t address, infra::ConstByteRange buffer);

        struct Chunk
        {
            Chunk(uint32_t alignedAddress, infra::ConstByteRange data)
                : alignedAddress(alignedAddress)
                , data(data)
            {}

            uint32_t alignedAddress;
            infra::ConstByteRange data;
        };

        Chunk* First();
        Chunk* Next();

    private:
        uint32_t AlignFirst(uint32_t address, infra::ConstByteRange& buffer);
        uint32_t AlignFullWords(uint32_t address, infra::ConstByteRange& buffer);
        void AlignLast(uint32_t address, infra::ConstByteRange& buffer);

        uint32_t alignedAddress = 0;

        infra::ByteRange firstChunk;
        infra::ByteRange lastChunk;
        uint8_t align;

        infra::ConstByteRange buffer;

        infra::BoundedDeque<Chunk>::WithMaxSize<3> chunks;
    };
}

#endif
