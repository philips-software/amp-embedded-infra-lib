#include "infra/util/ByteRange.hpp"
#include "infra/util/Crc.hpp"
#include "infra/util/CrcCcittCalculator.hpp"
#include "gtest/gtest.h"
#include <array>

/// Standard input sequence used to calculate the "check value"
static constexpr std::array<uint8_t, 9> checkInput = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };

TEST(TestCrc, PolyReverse)
{
    EXPECT_EQ(0x01, infra::PolyReverse<uint8_t>(0x80));
    EXPECT_EQ(0x1021, infra::PolyReverse<uint16_t>(0x8408));
    EXPECT_EQ(0xff001021, infra::PolyReverse<uint32_t>(0x840800ff));
}

TEST(TestCrc, Crc8Maxim)
{
    infra::Crc8Maxim crc;
    crc.Update(checkInput);
    EXPECT_EQ(0xa1, crc.Result());
}

TEST(TestCrc, Crc8MaximWithPlainArray)
{
    infra::Crc8Maxim crc;
    crc.Update(infra::MakeConstByteRange(checkInput));
    EXPECT_EQ(0xa1, crc.Result());
}

TEST(TestCrc, Crc8Sensirion)
{
    infra::Crc8Sensirion crc;
    crc.Update(checkInput);
    EXPECT_EQ(0xf7, crc.Result());
}

TEST(TestCrc, Crc16Modbus)
{
    infra::Crc16Modbus crc;
    crc.Update(checkInput);
    EXPECT_EQ(0x4b37, crc.Result());
}

TEST(TestCrc, Crc16Xmodem)
{
    infra::Crc16Xmodem crc;
    crc.Update(checkInput);
    EXPECT_EQ(0x31c3, crc.Result());
}

TEST(TestCrc, Crc16CcittFalse)
{
    infra::Crc16CcittFalse crc;
    crc.Update(checkInput);
    EXPECT_EQ(0x29B1, crc.Result());
}

TEST(TestCrc, Crc32)
{
    infra::Crc32 crc;
    crc.Update(checkInput);
    EXPECT_EQ(0xCBF43926, crc.Result());
}

TEST(TestCrc, Reset)
{
    infra::Crc16Modbus crc;
    crc.Update(1);
    crc.Update(2);
    crc.Update(3);
    crc.Reset();
    EXPECT_EQ(0xffff, crc.Result());
}
