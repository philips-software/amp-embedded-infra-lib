#include "infra/util/ByteRange.hpp"
#include "services/util/EllipticCurveDiffieHellman.hpp"
#include "services/util/test_doubles/EllipticCurveMock.hpp"
#include <gmock/gmock.h>

namespace
{
    class EllipticCurveDiffieHellmanTest
        : public testing::Test
    {
    public:
        testing::StrictMock<application::EllipticCurveOperationsMock> eccMock;
        application::EllipticCurveExtendedParameters curveStub;
        application::EllipticCurveDiffieHellman ecdh{ eccMock };
        // clang-format off
        std::array<uint8_t, 32> privateKey{
            0x4e, 0xdb, 0xb1, 0x09, 0x27, 0x68, 0xac, 0x3c,
            0x25, 0xe9, 0x55, 0x8a, 0xdb, 0xfe, 0xf6, 0xf9,
            0x36, 0x0e, 0x5f, 0x17, 0x6c, 0xef, 0xf6, 0xe4,
            0xce, 0xd6, 0x81, 0x55, 0x81, 0x9e, 0x51, 0xea,
        };
        std::array<uint8_t, 64> peerPublicKey{
            0x86, 0x0c, 0x77, 0x2a, 0x15, 0x5b, 0x53, 0xdb,
            0x29, 0xbf, 0x4c, 0xdd, 0x8a, 0x75, 0x98, 0xd9,
            0x29, 0xaf, 0x85, 0xe0, 0x89, 0x61, 0xf1, 0xd1,
            0x7f, 0x17, 0x71, 0xed, 0xa5, 0xf0, 0xd3, 0x8e,
            0x4e, 0x8d, 0x83, 0x6b, 0xec, 0xa0, 0xf9, 0x80,
            0x10, 0x93, 0x09, 0xbd, 0xde, 0x25, 0xc8, 0x41,
            0x34, 0x88, 0x1a, 0x27, 0xf4, 0x6b, 0x65, 0xc0,
            0x4d, 0x69, 0x60, 0x44, 0x91, 0x38, 0x25, 0x92,
        };
        std::array<uint8_t, 32> sharedSecretKeyExpected{
            0x09, 0x38, 0x2f, 0xbb, 0xcb, 0x50, 0x6f, 0xbc,
            0x34, 0x45, 0x12, 0xd1, 0x58, 0xc2, 0xb9, 0x04,
            0xf3, 0x0f, 0x7f, 0xa4, 0x71, 0x1e, 0x84, 0x0c,
            0x91, 0xd8, 0x5a, 0x71, 0x85, 0xc3, 0xc1, 0xbf,
        };
        // clang-format on
        std::array<uint8_t, 32> sharedSecretKeyGenerated{};

        infra::ConstByteRange peerPublicKeyX = infra::Head(infra::MakeRange(peerPublicKey), peerPublicKey.size() / 2);
        infra::ConstByteRange peerPublicKeyY = infra::Tail(infra::MakeRange(peerPublicKey), peerPublicKey.size() / 2);
    };

    MATCHER_P(ArrayContentsEqual, x, negation ? "Contents not equal" : "Contents are equal")
    {
        return x.size() == arg.size() && std::equal(x.begin(), x.end(), arg.begin());
    }
}

TEST_F(EllipticCurveDiffieHellmanTest, public_key_x_invalid)
{
    infra::Function<void(application::EllipticCurveOperations::ComparisonResult)> onDone;

    ::testing::InSequence _;

    EXPECT_CALL(eccMock, Comparison(ArrayContentsEqual(this->peerPublicKeyX), ::testing::_, ::testing::_))
        .WillOnce(::testing::SaveArg<2>(&onDone));

    ecdh.CalculateSharedSecretKey(curveStub, privateKey, peerPublicKey, sharedSecretKeyGenerated, [this](bool result)
        {
            EXPECT_FALSE(result);
        });
    onDone(application::EllipticCurveOperations::ComparisonResult::aGreaterThanB);
}

TEST_F(EllipticCurveDiffieHellmanTest, public_key_y_invalid)
{
    infra::Function<void(application::EllipticCurveOperations::ComparisonResult)> onPublicKeyXCalculated;
    infra::Function<void(application::EllipticCurveOperations::ComparisonResult)> onPublicKeyYCalculated;

    ::testing::InSequence _;

    EXPECT_CALL(eccMock, Comparison(ArrayContentsEqual(this->peerPublicKeyX), ::testing::_, ::testing::_))
        .WillOnce(::testing::SaveArg<2>(&onPublicKeyXCalculated));

    EXPECT_CALL(eccMock, Comparison(ArrayContentsEqual(this->peerPublicKeyY), ::testing::_, ::testing::_))
        .WillOnce(::testing::SaveArg<2>(&onPublicKeyYCalculated));

    ecdh.CalculateSharedSecretKey(curveStub, privateKey, peerPublicKey, sharedSecretKeyGenerated, [this](bool result)
        {
            EXPECT_FALSE(result);
        });
    onPublicKeyXCalculated(application::EllipticCurveOperations::ComparisonResult::aLessThanB);
    onPublicKeyYCalculated(application::EllipticCurveOperations::ComparisonResult::aGreaterThanB);
}

TEST_F(EllipticCurveDiffieHellmanTest, public_key_not_on_curve)
{
    infra::Function<void(application::EllipticCurveOperations::ComparisonResult)> onPublicKeyXCalculated;
    infra::Function<void(application::EllipticCurveOperations::ComparisonResult)> onPublicKeyYCalculated;
    infra::Function<void(application::EllipticCurveOperations::PointOnCurveResult)> onCheckPointOnCurveFinished;

    ::testing::InSequence _;

    EXPECT_CALL(eccMock, Comparison(ArrayContentsEqual(this->peerPublicKeyX), ::testing::_, ::testing::_))
        .WillOnce(::testing::SaveArg<2>(&onPublicKeyXCalculated));

    EXPECT_CALL(eccMock, Comparison(ArrayContentsEqual(this->peerPublicKeyY), ::testing::_, ::testing::_))
        .WillOnce(::testing::SaveArg<2>(&onPublicKeyYCalculated));

    EXPECT_CALL(eccMock, CheckPointOnCurve(::testing::_, ArrayContentsEqual(this->peerPublicKeyX), ArrayContentsEqual(this->peerPublicKeyY), ::testing::_))
        .WillOnce(::testing::SaveArg<3>(&onCheckPointOnCurveFinished));

    ecdh.CalculateSharedSecretKey(curveStub, privateKey, peerPublicKey, sharedSecretKeyGenerated, [this](bool result)
        {
            EXPECT_FALSE(result);
        });
    onPublicKeyXCalculated(application::EllipticCurveOperations::ComparisonResult::aLessThanB);
    onPublicKeyYCalculated(application::EllipticCurveOperations::ComparisonResult::aLessThanB);
    onCheckPointOnCurveFinished(application::EllipticCurveOperations::PointOnCurveResult::pointNotOnCurve);
}

TEST_F(EllipticCurveDiffieHellmanTest, calculate_shared_secret_key_successfully)
{
    infra::Function<void(application::EllipticCurveOperations::ComparisonResult)> onPublicKeyXCalculated;
    infra::Function<void(application::EllipticCurveOperations::ComparisonResult)> onPublicKeyYCalculated;
    infra::Function<void(application::EllipticCurveOperations::PointOnCurveResult)> onCheckPointOnCurveFinished;
    infra::Function<void(infra::ConstByteRange, infra::ConstByteRange)> onScalarMultiplicationFinished;

    ::testing::InSequence _;

    EXPECT_CALL(eccMock, Comparison(ArrayContentsEqual(this->peerPublicKeyX), ::testing::_, ::testing::_))
        .WillOnce(::testing::SaveArg<2>(&onPublicKeyXCalculated));

    EXPECT_CALL(eccMock, Comparison(ArrayContentsEqual(this->peerPublicKeyY), ::testing::_, ::testing::_))
        .WillOnce(::testing::SaveArg<2>(&onPublicKeyYCalculated));

    EXPECT_CALL(eccMock, CheckPointOnCurve(::testing::_, ArrayContentsEqual(this->peerPublicKeyX), ArrayContentsEqual(this->peerPublicKeyY), ::testing::_))
        .WillOnce(::testing::SaveArg<3>(&onCheckPointOnCurveFinished));

    EXPECT_CALL(eccMock, ScalarMultiplication(::testing::_, ArrayContentsEqual(this->privateKey), ArrayContentsEqual(this->peerPublicKeyX), ArrayContentsEqual(this->peerPublicKeyY), ::testing::_))
        .WillOnce(::testing::SaveArg<4>(&onScalarMultiplicationFinished));

    ecdh.CalculateSharedSecretKey(curveStub, privateKey, peerPublicKey, sharedSecretKeyGenerated, [this](bool result)
        {
            EXPECT_TRUE(result);
            EXPECT_TRUE(sharedSecretKeyGenerated == sharedSecretKeyExpected);
        });
    onPublicKeyXCalculated(application::EllipticCurveOperations::ComparisonResult::aLessThanB);
    onPublicKeyYCalculated(application::EllipticCurveOperations::ComparisonResult::aLessThanB);
    onCheckPointOnCurveFinished(application::EllipticCurveOperations::PointOnCurveResult::pointOnCurve);
    onScalarMultiplicationFinished(infra::MakeRange(this->sharedSecretKeyExpected), infra::ConstByteRange());
}
