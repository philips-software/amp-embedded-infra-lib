#include "infra/util/ByteRange.hpp"
#include "infra/util/Crc.hpp"
#include "gtest/gtest.h"
#include <array>

/// Standard input sequence used to calculate the "check value"
static constexpr std::array<uint8_t, 9> checkInput = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };

TEST(TestCrc, PolyReverse)
{
    static_assert(0x01 == infra::PolyReverse<uint8_t>(0x80), "PolyReverse<uint8_t> failed");
    static_assert(0x1021 == infra::PolyReverse<uint16_t>(0x8408), "PolyReverse<uint16_t> failed");
    static_assert(uint32_t{ 0xff001021 } == infra::PolyReverse<uint32_t>(0x840800ff), "PolyReverse<uint32_t> failed");
}

TEST(TestCrc, Crc8Maxim)
{
    constexpr auto Crc8Maxim = []
    {
        infra::Crc8Maxim crc;
        crc.Update(checkInput);
        return crc.Result();
    };
    static_assert(0xa1 == Crc8Maxim(), "Crc8Maxim failed");
}

TEST(TestCrc, Crc8Sensirion)
{
    constexpr auto Crc8Sensirion = []
    {
        infra::Crc8Sensirion crc;
        crc.Update(checkInput);
        return crc.Result();
    };
    static_assert(0xf7 == Crc8Sensirion(), "Crc8Sensirion failed");
}

TEST(TestCrc, Crc16Modbus)
{
    constexpr auto Crc16Modbus = []
    {
        infra::Crc16Modbus crc;
        crc.Update(checkInput);
        return crc.Result();
    };
    static_assert(0x4b37 == Crc16Modbus(), "Crc16Modbus failed");
}

TEST(TestCrc, Crc16Xmodem)
{
    constexpr auto Crc16Xmodem = []
    {
        infra::Crc16Xmodem crc;
        crc.Update(checkInput);
        return crc.Result();
    };
    static_assert(0x31c3 == Crc16Xmodem(), "Crc16Xmodem failed");
}

TEST(TestCrc, Crc16CcittFalse)
{
    constexpr auto Crc16CcittFalse = []
    {
        infra::Crc16CcittFalse crc;
        crc.Update(checkInput);
        return crc.Result();
    };
    static_assert(0x29B1 == Crc16CcittFalse(), "Crc16CcittFalse failed");
}

TEST(TestCrc, Crc32)
{
    constexpr auto Crc32 = []
    {
        infra::Crc32 crc;
        crc.Update(checkInput);
        return crc.Result();
    };
    static_assert(0xCBF43926 == Crc32(), "Crc32 failed");
}

TEST(TestCrc, Reset)
{
    constexpr auto Reset = []
    {
        infra::Crc16Modbus crc;
        crc.Update(1);
        crc.Update(2);
        crc.Update(3);
        crc.Reset();
        return crc.Result();
    };
    static_assert(0xffff == Reset(), "Reset failed");
}
