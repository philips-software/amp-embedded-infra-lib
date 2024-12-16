#include "infra/util/ByteRange.hpp"
#include "infra/util/Crc.hpp"
#include "infra/util/CrcCcittCalculator.hpp"
#include "gtest/gtest.h"
#include <array>

/// Standard input sequence used to calculate the "check value"
static constexpr std::array<uint8_t, 9> checkInput = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };

TEST(TestCrcCcittCalculator, CrcCcittCalculator)
{
    infra::CrcCcittCalculator crcCcitt;
    crcCcitt.Update(checkInput);
    EXPECT_EQ(0X29b1, crcCcitt.Result());
}
