#include "upgrade/boot_loader/DecryptorAesTiny.hpp"

extern "C"
{
#include "TinyAes.h"
}

namespace application
{
    DecryptorAesTiny::DecryptorAesTiny(infra::ConstByteRange key)
        : currentStreamBlock()
        , counter()
    {
        AES128_ECB_encrypt(counter.data(), key.begin(), currentStreamBlock.data()); // Dummy encryption to trigger key expansion
    }

    infra::ByteRange DecryptorAesTiny::StateBuffer()
    {
        return infra::MakeByteRange(counter);
    }

    void DecryptorAesTiny::Reset()
    {
        currentStreamBlockOffset = 0;
    }

    void DecryptorAesTiny::DecryptPart(infra::ByteRange data)
    {
        while (!data.empty())
        {
            if (currentStreamBlockOffset == 0)
            {
                AES128_ECB_encrypt(counter.data(), nullptr, currentStreamBlock.data());
                IncreaseCounter();
            }

            data.front() ^= currentStreamBlock[currentStreamBlockOffset];
            data.pop_front();
            ++currentStreamBlockOffset;

            if (currentStreamBlockOffset == currentStreamBlock.size())
                currentStreamBlockOffset = 0;
        }
    }

    void DecryptorAesTiny::IncreaseCounter()
    {
        for (std::size_t i = counter.size(); i != 0; --i)
            if (++counter[i - 1] != 0)
                break;
    }

    bool DecryptorAesTiny::DecryptAndAuthenticate(infra::ByteRange data)
    {
        DecryptPart(data);

        return true;
    }
}
