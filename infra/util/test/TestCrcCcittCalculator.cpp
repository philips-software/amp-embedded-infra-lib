#include "infra/util/ByteRange.hpp"
#include "infra/util/CrcCcittCalculator.hpp"
#include "gtest/gtest.h"
#include <array>

/// Standard input sequence used to calculate the "check value"
static constexpr std::array<uint8_t, 9> checkInput = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };

TEST(TestCrcCcittCalculator, CrcCcittCalculator)
{
    constexpr auto CrcCcittCalculator = []
    {
        infra::CrcCcittCalculator crc;
        crc.Update(checkInput);
        return crc.Result();
    };
    static_assert(0x29B1 == CrcCcittCalculator(), "CrcCcittCalculator failed");
}
