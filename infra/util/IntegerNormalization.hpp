#ifndef INFRA_INTEGER_NORMALIZATION_HPP
#define INFRA_INTEGER_NORMALIZATION_HPP

#include <cstdint>
#include <limits.h>
#include <type_traits>

namespace infra
{
    namespace detail
    {
        template<std::size_t N>
        struct SignedInteger;
        template<>
        struct SignedInteger<8>
        {
            using type = int8_t;
        };
        template<>
        struct SignedInteger<16>
        {
            using type = int16_t;
        };
        template<>
        struct SignedInteger<32>
        {
            using type = int32_t;
        };
        template<>
        struct SignedInteger<64>
        {
            using type = int64_t;
        };

        template<std::size_t N>
        struct UnsignedInteger;
        template<>
        struct UnsignedInteger<8>
        {
            using type = uint8_t;
        };
        template<>
        struct UnsignedInteger<16>
        {
            using type = uint16_t;
        };
        template<>
        struct UnsignedInteger<32>
        {
            using type = uint32_t;
        };
        template<>
        struct UnsignedInteger<64>
        {
            using type = uint64_t;
        };

        template<class T>
        struct NormalizedSignedInteger
        {
            using type = typename SignedInteger<CHAR_BIT * sizeof(T)>::type;

            static_assert(std::is_integral<T>::value, "T is not an integral type");
            static_assert(sizeof(type) == sizeof(T), "Size mismatch between T and deduced type");
            static_assert(std::is_signed<type>::value, "Signedness mismatch between T and deduced type");
        };

        template<class T>
        struct NormalizedUnsignedInteger
        {
            using type = typename UnsignedInteger<CHAR_BIT * sizeof(T)>::type;

            static_assert(std::is_integral<T>::value, "T is not an integral type");
            static_assert(sizeof(type) == sizeof(T), "Size mismatch between T and deduced type");
            static_assert(std::is_unsigned<type>::value, "Signedness mismatch between T and deduced type");
        };
    }

    template<class T, bool = std::is_enum<T>::value>
    struct NormalizedIntegralType;

    template<class T>
    struct NormalizedIntegralType<T, false>
    {
        using type = typename std::conditional<std::is_signed<T>::value,
            typename detail::NormalizedSignedInteger<T>::type,
            typename detail::NormalizedUnsignedInteger<T>::type>::type;
    };

    template<class T>
    struct NormalizedIntegralType<T, true>
    {
        using type = typename NormalizedIntegralType<typename std::underlying_type<T>::type, false>::type;
    };
}

#endif
