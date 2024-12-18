#include "services/util/AesCmac.hpp"
#include <algorithm>

namespace
{
    uint32_t Reverse(uint32_t data)
    {
        return ((data & 0x000000FF) << 24) |
               ((data & 0x0000FF00) << 8) |
               ((data & 0x00FF0000) >> 8) |
               ((data & 0xFF000000) >> 24);
    }

    void KeyRoll(infra::ByteRange data)
    {
        auto key = infra::ReinterpretCastMemoryRange<uint32_t>(data);

        auto carry = ((key.front() >> 31) & 1) * 0x87;

        for (std::size_t i = 1; i != key.size(); i++)
            key[i - 1] = (key[i - 1] << 1) | (key[i] >> 31);

        key.back() = (key.back() << 1) ^ carry;
    }

    infra::ConstByteRange Xor(infra::ConstByteRange x, infra::ByteRange y)
    {
        auto data = infra::ConstCastMemoryRange<uint32_t>(infra::ReinterpretCastMemoryRange<const uint32_t>(x));
        auto key = infra::ReinterpretCastMemoryRange<uint32_t>(y);

        for (std::size_t i = 0; i != data.size(); i++)
            data[i] = Reverse(data[i]) ^ (key[i]);

        return infra::ReinterpretCastByteRange(data);
    }

    infra::ConstByteRange XorIncludingResult(infra::ByteRange x, infra::ByteRange y, infra::ByteRange result)
    {
        auto res = infra::ReinterpretCastMemoryRange<uint32_t>(result);
        auto tag = infra::ReinterpretCastMemoryRange<uint32_t>(x);
        auto key = infra::ReinterpretCastMemoryRange<uint32_t>(y);

        for (std::size_t i = 0; i < res.size(); i++)
            res[i] ^= tag[i] ^ key[i];

        return infra::ReinterpretCastByteRange(res);
    }

    void AddByteBigEndianFormat(infra::ByteRange input, std::size_t position, uint32_t data)
    {
        auto inputRange = infra::ReinterpretCastMemoryRange<uint32_t>(input);

        inputRange[position / 4] |= data << (8 * (3 - (position % 4)));
    }

    std::size_t CalculateRemainingBytes(std::size_t size, std::size_t blockSize)
    {
        auto remainingBytes = size % blockSize;
        if ((size != 0) && (remainingBytes == 0))
            remainingBytes = 16;

        return remainingBytes;
    }

    void CopyReversedWords(infra::ByteRange destination, infra::ByteRange source)
    {
        auto dest = infra::ReinterpretCastMemoryRange<uint32_t>(destination);
        auto src = infra::ReinterpretCastMemoryRange<uint32_t>(source);

        for (std::size_t i = 0; i != dest.size(); ++i)
            dest[i] = Reverse(src[i]);
    }
}

namespace services
{
    Aes128CmacImpl::Aes128CmacImpl(Aes128Ecb& aes128Ecb)
        : aes128Ecb(aes128Ecb)
    {}

    void Aes128CmacImpl::SetKey(const std::array<uint8_t, 16>& key)
    {
        lastBlockSize = 0;
        tag.fill(0);
        aes128Ecb.SetKey(key);
    }

    void Aes128CmacImpl::Append(infra::ConstByteRange input)
    {
        std::array<uint8_t, blockSize> chunk{};
        lastBlockSize = CalculateRemainingBytes(input.size(), blockSize);
        std::copy(input.end() - lastBlockSize, input.end(), lastBlock.begin());

        input = infra::DiscardTail(input, lastBlockSize);

        while (!input.empty())
        {
            std::copy(input.begin(), input.begin() + blockSize, chunk.begin());
            aes128Ecb.Encrypt(Xor(chunk, tag), tag);
            input = infra::DiscardHead(input, blockSize);
        }
    }

    Aes128Cmac::Mac Aes128CmacImpl::Calculate()
    {
        std::array<uint8_t, blockSize> subkey{};
        std::array<uint8_t, blockSize> temp{};

        for (std::size_t i = 0; i != lastBlockSize; ++i)
            AddByteBigEndianFormat(temp, i, lastBlock[i]);

        ComputeK1(subkey);

        if (lastBlockSize < blockSize)
            ComputeK2(temp, subkey);

        aes128Ecb.Encrypt(XorIncludingResult(tag, subkey, temp), tag);

        CopyReversedWords(temp, tag);
        tag = temp;

        return tag;
    }

    void Aes128CmacImpl::ComputeK1(infra::ByteRange key)
    {
        aes128Ecb.Encrypt(key, key);
        KeyRoll(key);
    }

    void Aes128CmacImpl::ComputeK2(infra::ByteRange block, infra::ByteRange key)
    {
        AddByteBigEndianFormat(block, lastBlockSize, 0x80);
        KeyRoll(key);
    }
}
