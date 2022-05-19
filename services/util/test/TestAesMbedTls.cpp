#include "gtest/gtest.h"
#include "services/util/AesMbedTls.hpp"

class Aes128CtrMbedTlsTest
    : public testing::Test
{
public:
    Aes128CtrMbedTlsTest()
    {
        aes128CtrMbedTls.SetKey(key);
    }

    std::array<uint8_t, 16> key{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    std::array<uint8_t, 16> counter{ 0 };
    std::array<uint8_t, 4> input{ 1, 2, 3, 4 };
    std::array<uint8_t, 4> output{ 0 };
    services::Aes128CtrMbedTls aes128CtrMbedTls;
};

TEST_F(Aes128CtrMbedTlsTest, should_encrypt_input)
{
    aes128CtrMbedTls.Encrypt(counter, infra::MakeConstByteRange(input), infra::MakeByteRange(output));

    EXPECT_EQ((std::array<uint8_t, 4>{
        0xc7, 0xa3, 0x38, 0x33
    }), output);
}

TEST_F(Aes128CtrMbedTlsTest, should_return_different_result_for_same_input)
{
    aes128CtrMbedTls.Encrypt(counter, infra::MakeConstByteRange(input), infra::MakeByteRange(output));
    aes128CtrMbedTls.Encrypt(counter, infra::MakeConstByteRange(input), infra::MakeByteRange(output));

    EXPECT_EQ((std::array<uint8_t, 4>{
        0x72, 0x44, 0x10, 0x91
    }), output);
}
