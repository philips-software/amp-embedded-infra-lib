#ifndef INFRA_CRC_HPP
#define INFRA_CRC_HPP

#include "infra/util/ByteRange.hpp"
#include <climits>
#include <cstddef>
#include <cstdint>

namespace infra
{
    /// Generic class template for different CRC calculations.
    ///
    /// Some ideas borrowed from http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
    /// Also useful:
    /// - http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
    /// - https://crccalc.com/

    template<typename CRC_TYPE>
    constexpr CRC_TYPE PolyReverse(CRC_TYPE Polynomial)
    {
        CRC_TYPE ret = 0;
        CRC_TYPE maskIn = 1;
        CRC_TYPE maskOut = 1 << (sizeof(CRC_TYPE) * 8 - 1);
        while (maskIn)
        {
            if (Polynomial & maskIn)
                ret |= maskOut;
            maskOut >>= 1;
            maskIn <<= 1;
        }
        return ret;
    }

    template<typename CRC_TYPE, CRC_TYPE Polynomial, CRC_TYPE InitValue = 0, CRC_TYPE FinalXor = 0, bool ReflectInput = false, bool ReflectOutput = false>
    class Crc
    {
        static_assert(ReflectInput == ReflectOutput, "Currently, non-matching ReflectInput and ReflectOutput is not supported");

    public:
        Crc() = default;
        ~Crc() = default;

        /// Update the CRC calculation with the next input byte.
        void Update(uint8_t input)
        {
            // Note this is a compile-time check, as ReflectInput is a template parameter
            if (ReflectInput)
            {
                // Update the LSB of crc value with next input byte
                crc ^= input;
                // This byte value is the index into the lookup table, make sure it's a byte
                uint8_t pos = crc & 0xFF;
                // Shift out this index
                if (bitWidthEquals8)
                    crc = 0;
                else
                    crc >>= 8;
                // XOR-in remainder from lookup table using the calculated index
                crc ^= table[pos];
            }
            else
            {
                // Update the MSB of crc value with next input byte
                crc ^= (CRC_TYPE(input) << shift);
                // This MSB byte value is the index into the lookup table
                uint8_t pos = static_cast<uint8_t>(crc >> shift);
                // Shift out this index
                if (bitWidthEquals8)
                    crc = 0;
                else
                    crc <<= 8;
                // XOR-in remainder from lookup table using the calculated index
                crc ^= table[pos];
            }
        }

        /// Update the CRC calculation with the given bytes as input.
        void Update(ConstByteRange bytes)
        {
            for (auto byte : bytes)
                Update(byte);
        }

        /// Get the final result.
        CRC_TYPE Result() const
        {
            return crc ^ FinalXor;
        }

        /// Reset the CRC to its initial value.
        void Reset()
        {
            crc = InitValue;
        }

    private:
        /// CRC lookup table which can be used as a const(expr) member.
        class Table
        {
        public:
            constexpr Table()
                : table()
            {
                // Initialize the table.
                for (unsigned int i = 0; i < 256; ++i)
                {
                    auto input = static_cast<uint8_t>(i);
                    if (ReflectInput)
                        input = Reflect(input);

                    // Pad input as needed
                    CRC_TYPE crcEntry = static_cast<CRC_TYPE>(input) << shift;

                    for (int bit = 0; bit < CHAR_BIT; ++bit)
                    {
                        constexpr CRC_TYPE msb = CRC_TYPE(1) << (bitWidth - 1);
                        if ((crcEntry & msb) != 0)
                        {
                            crcEntry <<= 1;
                            crcEntry ^= Polynomial;
                        }
                        else
                            crcEntry <<= 1;
                    }

                    if (ReflectOutput)
                        table[i] = Reflect(crcEntry);
                    else
                        table[i] = crcEntry;
                }
            }

            CRC_TYPE operator[](int i) const
            {
                return table[i];
            }

        private:
            CRC_TYPE table[256];
        };

        static constexpr uint8_t bitWidth = sizeof(CRC_TYPE) * CHAR_BIT;
        static constexpr bool bitWidthEquals8 = bitWidth == 8;
        static constexpr uint8_t shift = bitWidth - 8;

        /// Revert bit order of the given value.
        template<typename TYPE>
        static constexpr TYPE Reflect(TYPE value)
        {
            TYPE result = 0;
            constexpr size_t bitsToReflect = sizeof(TYPE) * CHAR_BIT;

            for (size_t i = 0; i < bitsToReflect; ++i, value >>= 1)
            {
                result = (result << 1) | (value & 0x01);
            }

            return result;
        }

        /// The latest calculated value, excluding any final XOR'ing.
        CRC_TYPE crc = InitValue;

        /// The lookup table.
        static inline constexpr Table table{};
    };

    using Crc8Maxim = Crc<uint8_t, 0x31, 0, 0, true, true>;
    using Crc8Sensirion = Crc<uint8_t, 0x31, 0xff, 0, false, false>;
    using Crc16Modbus = Crc<uint16_t, 0x8005, 0xffff, 0, true, true>;
    using Crc16Xmodem = Crc<uint16_t, 0x1021, 0x0>;
    using Crc16CcittFalse = Crc<uint16_t, 0x1021, 0xffff>;
    using Crc64Ecma = Crc<uint64_t, 0x42F0E1EBA9EA3693, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, true, true>;
}

#endif
