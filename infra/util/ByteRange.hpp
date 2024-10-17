#ifndef INFRA_BYTE_RANGE_HPP
#define INFRA_BYTE_RANGE_HPP

#include "infra/util/MemoryRange.hpp"
#include <cstdint>
#include <cstring>
#include <ostream>
#include <string>

namespace infra
{
    using ByteRange = MemoryRange<uint8_t>;
    using ConstByteRange = MemoryRange<const uint8_t>;

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
    std::string ByteRangeAsStdString(infra::ConstByteRange range);

#ifdef EMIL_HOST_BUILD
    // gtest uses PrintTo to display the contents of ByteRange
    inline void PrintTo(ByteRange range, std::ostream* os)
    {
        *os << "ByteRange size " << range.size() << " contents [";
        for (uint8_t& e : infra::Head(range, 8))
        {
            *os << static_cast<int>(e);
            if (std::distance(range.begin(), &e) != static_cast<int32_t>(range.size()) - 1)
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
            if (std::distance(range.begin(), &e) != static_cast<int32_t>(range.size()) - 1)
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
        return ReinterpretCastByteRange(MakeRange(string, string + std::strlen(string))); //NOSONAR
    }

    inline ConstByteRange MakeStringByteRange(const std::string& string)
    {
        return ReinterpretCastByteRange(MakeRange(string.data(), string.data() + string.size()));
    }

    inline std::string ByteRangeAsStdString(infra::ConstByteRange range)
    {
        return std::string(reinterpret_cast<const char*>(range.begin()), range.size());
    }
}

#endif
