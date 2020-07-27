#ifndef INFRA_BIT_LOGIC_HPP
#define INFRA_BIT_LOGIC_HPP

#include <cstdint>
#include <limits>
#include <type_traits>

namespace infra
{
    template<class T>
    constexpr typename std::remove_cv<T>::type Bit(uint8_t index)
    {
        return static_cast<typename std::remove_cv<T>::type>(1) << index;
    }

    template<class T>
    void SetBit(T& dataRegister, uint8_t index)
    {
        dataRegister |= Bit<T>(index);
    }

    template<class T>
    void ClearBit(T& dataRegister, uint8_t index)
    {
        dataRegister &= ~Bit<T>(index);
    }

    template<class T>
    void ReplaceBit(T& dataRegister, bool newValue, uint8_t position)
    {
        dataRegister = (dataRegister & ~Bit<T>(position)) | (static_cast<T>(newValue) << position);
    }

    template<class T>
    void ReplaceBit(T& dataRegister, typename std::remove_cv<T>::type newValue, uint8_t position)
    {
        dataRegister = (dataRegister & ~Bit<T>(position)) | (newValue << position);
    }

    template<class T>
    void ReplaceBits(T& dataRegister, uint8_t size, typename std::remove_cv<T>::type newValue, uint8_t position)
    {
        dataRegister = (dataRegister & ~(~(std::numeric_limits<typename std::remove_cv<T>::type>::max() << size) << (position * size))) | (newValue << (position * size));
    }

    template<class T>
    bool IsBitSet(T dataRegister, uint8_t index)
    {
        return (dataRegister & Bit<T>(index)) != 0;
    }

    template<class T>
    T GetBits(T dataRegister, uint8_t size, uint8_t position)
    {
        return dataRegister & (~(std::numeric_limits<typename std::remove_cv<T>::type>::max() << size) << (position * size));
    }

    template<class T>
    void MaskedUpdate(T& dataRegister, typename std::remove_cv<T>::type mask, typename std::remove_cv<T>::type update)
    {
        dataRegister = (dataRegister & ~mask) | update;
    }
}

#endif
