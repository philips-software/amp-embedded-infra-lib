#include "infra/util/MemoryRange.hpp"
#include "services/util/KeyGenerator.hpp"
#include "services/util/test_doubles/EllipticCurveMock.hpp"
#include <gmock/gmock.h>

namespace
{
    class KeyGenerationTest
        : public testing::Test
    {
    public:
        testing::StrictMock<services::EllipticCurveOperationsMock> eccMock;
        services::EllipticCurveExtendedParameters curveStub;
        services::KeyGenerator keyGenerator{ eccMock };
        // clang-format off
        std::array<uint8_t, 32> privateKey{
            0xbd, 0x46, 0x7c, 0x77, 0xf8, 0xb5, 0xf9, 0xff,
            0xb7, 0xae, 0x72, 0x70, 0xd4, 0xbb, 0x2b, 0xbf,
            0xf6, 0x0d, 0x44, 0x5a, 0xbf, 0x5f, 0xda, 0x32,
            0x70, 0x31, 0x6a, 0x63, 0xa4, 0x6c, 0x69, 0x07
        };
        std::array<uint8_t, 64> publicKeyExpected{
            0x6f, 0x09, 0xec, 0x49, 0x10, 0xc3, 0xd8, 0xb8,
            0xf6, 0x3d, 0x76, 0x35, 0xcf, 0xf9, 0x5c, 0x58,
            0x01, 0x6a, 0xbb, 0x5b, 0x87, 0x31, 0xcf, 0x72,
            0xf6, 0x01, 0xb6, 0x21, 0x88, 0xf1, 0x4e, 0x48,
            0xd4, 0x0d, 0x26, 0x03, 0x06, 0x6f, 0x87, 0x44,
            0x8c, 0xed, 0xd9, 0x4a, 0x1f, 0xed, 0x61, 0x78,
            0x33, 0xdd, 0xb5, 0x56, 0x8c, 0x4b, 0x7a, 0x75,
            0xd9, 0x6e, 0x4a, 0x6d, 0x72, 0xa3, 0xbf, 0x5a,
        };
        // clang-format on
        std::array<uint8_t, 64> publicKeyGenerated{};
    };
}

TEST_F(KeyGenerationTest, generate_public_key_from_private_key)
{
    EXPECT_CALL(eccMock, ScalarMultiplication(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(::testing::Invoke([this](auto& curve, auto scalar, auto x, auto y, auto& onDone)
            {
                auto result = infra::MakeRange(publicKeyExpected);

                EXPECT_EQ(32, scalar.size());
                EXPECT_TRUE(infra::ContentsEqual(scalar, infra::MakeRange(this->privateKey)));
                EXPECT_TRUE(x.empty());
                EXPECT_TRUE(y.empty());
                onDone(infra::Head(result, 32), infra::Tail(result, 32));
            }));

    keyGenerator.GeneratePublicKeyFromPrivateKey(curveStub, privateKey, publicKeyGenerated, [this]()
        {
            EXPECT_TRUE(publicKeyGenerated == publicKeyExpected);
        });
}
