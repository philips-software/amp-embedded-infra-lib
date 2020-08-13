#ifndef INFRA_ENDIAN_HPP
#define INFRA_ENDIAN_HPP

#include "infra/util/ByteRange.hpp"
#include <iterator>
#include <type_traits>

namespace infra
{
    // For now, we assume host endianness is little endian

    namespace detail
    {
        // System Workbench's GCC does not yet support std::make_reverse_iterator
        template<class Iterator>
        std::reverse_iterator<Iterator> make_reverse_iterator(Iterator i)
        {
            return std::reverse_iterator<Iterator>(i);
        }
    }

    template<class T>
    T SwapEndian(T value)
    {
        static_assert(std::is_standard_layout<T>::value, "T should be standard layout");

        T result;
        auto valueRange = infra::MakeByteRange(value);
        std::copy(valueRange.begin(), valueRange.end(), detail::make_reverse_iterator(infra::MakeByteRange(result).end()));
        return result;
    }

    template<class T>
    T FromBigEndian(T value)
    {
        return SwapEndian(value);
    }

    template<class T>
    T ToBigEndian(T value)
    {
        return SwapEndian(value);
    }

    template<class T>
    T FromLittleEndian(T value)
    {
        return value;
    }

    template<class T>
    T ToLittleEndian(T value)
    {
        return value;
    }

    template<class T>
    class BigEndian
    {
    public:
        BigEndian() = default;

        BigEndian(T value)
            : value(ToBigEndian(value))
        {}

        operator T() const
        {
            return FromBigEndian(value);
        }

        bool operator==(BigEndian other) const
        {
            return value == other.value;
        }

        bool operator!=(BigEndian other) const
        {
            return !(*this == other);
        }

        bool operator==(T other) const
        {
            return value == ToBigEndian(other);
        }

        bool operator!=(T other) const
        {
            return !(*this == other);
        }

        template<class U>
        bool operator==(U other) const
        {
            return value == ToBigEndian(static_cast<T>(other));
        }

        template<class U>
        bool operator!=(U other) const
        {
            return !(*this == other);
        }

        friend bool operator==(T x, BigEndian y)
        {
            return y == x;
        }

        friend bool operator!=(T x, BigEndian y)
        {
            return y != x;
        }

        template<class U>
        friend bool operator==(U x, BigEndian y)
        {
            return y == x;
        }

        template<class U>
        friend bool operator!=(U x, BigEndian y)
        {
            return y != x;
        }

    private:
        T value{};
    };

    template<class T>
    class LittleEndian
    {
    public:
        LittleEndian() = default;

        LittleEndian(T value)
            : value(ToLittleEndian(value))
        {}

        operator T() const
        {
            return FromLittleEndian(value);
        }

        bool operator==(LittleEndian other) const
        {
            return value == other.value;
        }

        bool operator!=(LittleEndian other) const
        {
            return !(*this == other);
        }

        bool operator==(T other) const
        {
            return value == ToLittleEndian(other);
        }

        bool operator!=(T other) const
        {
            return !(*this == other);
        }

        template<class U>
        bool operator==(U other) const
        {
            return value == ToLittleEndian(static_cast<T>(other));
        }

        template<class U>
        bool operator!=(U other) const
        {
            return !(*this == other);
        }

        friend bool operator==(T x, LittleEndian y)
        {
            return y == x;
        }

        friend bool operator!=(T x, LittleEndian y)
        {
            return y != x;
        }

        template<class U>
        friend bool operator==(U x, LittleEndian y)
        {
            return y == x;
        }

        template<class U>
        friend bool operator!=(U x, LittleEndian y)
        {
            return y != x;
        }

    private:
        T value{};
    };
}

#endif
