#ifndef INFRA_BYTE_RANGE_HPP
#define INFRA_BYTE_RANGE_HPP

#include "infra/util/MemoryRange.hpp"
#include <cstdint>
#include <cstring>
#include <ostream>
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

#ifdef CCOLA_HOST_BUILD //TICS !POR#021
    // gtest uses PrintTo to display the contents of ByteRange
    inline void PrintTo(ByteRange range, std::ostream* os)
    {
        *os << "ByteRange size " << range.size() << " contents [";
        for (uint8_t& e : infra::Head(range, 8))
        {
            *os << static_cast<int>(e);
            if (std::distance(range.begin(), &e) != range.size() - 1)
                *os << ", ";
        }
        if (range.size() > 8)
            *os << "...";
        *os << "]";
    }

    inline void PrintTo(ConstByteRange range, std::ostream* os)
    {
        *os << "ConstByteRange size " << range.size() << " contents [";
        for (const uint8_t& e : infra::Head(range, 8))
        {
            *os << static_cast<int>(e);
            if (std::distance(range.begin(), &e) != range.size() - 1)
                *os << ", ";
        }
        if (range.size() > 8)
            *os << "...";
        *os << "]";
    }
#endif

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
