#ifndef INFRA_BYTE_RANGE_HPP
#define INFRA_BYTE_RANGE_HPP

#include "infra/util/MemoryRange.hpp"
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace infra
{
    typedef MemoryRange<uint8_t> ByteRange;
    typedef MemoryRange<const uint8_t> ConstByteRange;

    template<class U>
        ByteRange ReinterpretCastByteRange(MemoryRange<U> memoryRange);
    template<class U>
        ConstByteRange ReinterpretCastByteRange(MemoryRange<const U> memoryRange);
    inline ByteRange ConstCastByteRange(ConstByteRange byteRange);

    template<class T>
        ByteRange MakeByteRange(T& v);
    template<class T>
        ConstByteRange MakeConstByteRange(T& v);
    template<class T>
        ConstByteRange MakeByteRange(const T& v);
    ConstByteRange MakeStringByteRange(const char* string);
    ConstByteRange MakeStringByteRange(const std::string& string);

    ////    Implementation    ////

    template<class U>
    ByteRange ReinterpretCastByteRange(MemoryRange<U> memoryRange)
    {
        return ReinterpretCastMemoryRange<uint8_t>(memoryRange);
    }

    template<class U>
    ConstByteRange ReinterpretCastByteRange(MemoryRange<const U> memoryRange)
    {
        return ReinterpretCastMemoryRange<const uint8_t>(memoryRange);
    }

    inline ByteRange ConstCastByteRange(ConstByteRange byteRange)
    {
        return ConstCastMemoryRange<uint8_t>(byteRange);
    }
        
    template<class T>
    ByteRange MakeByteRange(T& v)
    {
        return ReinterpretCastByteRange(MakeRange(&v, &v + 1));
    }

    template<class T>
    ConstByteRange MakeConstByteRange(T& v)
    {
        return ReinterpretCastByteRange(MakeRange(&v, &v + 1));
    }

    template<class T>
    ConstByteRange MakeByteRange(const T& v)
    {
        return ReinterpretCastByteRange(MakeRange(&v, &v + 1));
    }

    inline ConstByteRange MakeStringByteRange(const char* string)
    {
        return ReinterpretCastByteRange(MakeRange(string, string + std::strlen(string)));
    }

    inline ConstByteRange MakeStringByteRange(const std::string& string)
    {
        return ReinterpretCastByteRange(MakeRange(string.data(), string.data() + string.size()));
    }
}

#endif
