#include "hal/synchronous_interfaces/test_doubles/SynchronousFlashStub.hpp"
#include "upgrade/boot_loader/VerifierEcDsa.hpp"
#include "gmock/gmock.h"

class VerifierEcDsaTest224
    : public testing::Test
{
public:
    const std::array<uint8_t, 56> ecDsa224PublicKey{
        0x86, 0x9b, 0xa2, 0x7d, 0x39, 0x56, 0x43, 0x16,
        0x1c, 0xfd, 0x34, 0x7f, 0x9e, 0x21, 0x44, 0x88,
        0x21, 0x7e, 0xc5, 0x26, 0xfe, 0x5f, 0x1e, 0xca,
        0x50, 0x6b, 0xce, 0xe2, 0x6f, 0xf6, 0x9d, 0x3b,
        0x1d, 0xcc, 0x98, 0xe9, 0xee, 0x1e, 0xb4, 0xe8,
        0xf4, 0xa5, 0xae, 0x2f, 0x30, 0x6e, 0xab, 0x9b,
        0x7b, 0xb6, 0xbf, 0xe7, 0x3b, 0x16, 0x2a, 0x8e
    };

    application::VerifierEcDsa224 verifier{ infra::MakeConstByteRange(ecDsa224PublicKey) };
};

TEST_F(VerifierEcDsaTest224, incorrect_hash_size_is_invalid)
{
    hal::SynchronousFlashStub flash(1, 64);
    EXPECT_EQ(false, verifier.IsValid(flash, { 0, 0 }, { 0, 4 }));
}

TEST_F(VerifierEcDsaTest224, incorrect_hash_is_invalid)
{
    hal::SynchronousFlashStub flash(1, 64);

    flash.sectors = {
        { 0, 1, 2, 3 }
    };

    flash.sectors[0].insert(flash.sectors[0].end(), 56, 0xFF);

    EXPECT_EQ(false, verifier.IsValid(flash, { 4, 60 }, { 0, 4 }));
}

TEST_F(VerifierEcDsaTest224, correct_hash_as_signature_is_valid)
{
    hal::SynchronousFlashStub flash(1, 64);

    flash.sectors = {
        { 0, 1, 2, 3,
            0xbe, 0xe0, 0x71, 0x4a, 0xeb, 0xf1, 0x68, 0x2e,
            0xfe, 0x16, 0x22, 0xa9, 0x74, 0x66, 0x4e, 0x5b,
            0x71, 0xe4, 0xdf, 0xfc, 0x3c, 0xdd, 0x24, 0x12,
            0xb9, 0xa4, 0xb6, 0x2b, 0xdd, 0x7c, 0x7f, 0x96,
            0x7b, 0x1d, 0xd3, 0xf8, 0x82, 0x8e, 0xc2, 0x15,
            0x78, 0xa7, 0x73, 0x47, 0xef, 0x66, 0xc9, 0x89,
            0xe1, 0x6c, 0xcb, 0x22, 0x6d, 0x95, 0x9b, 0x44 }
    };

    EXPECT_EQ(true, verifier.IsValid(flash, { 4, 60 }, { 0, 4 }));
}

class VerifierEcDsaTest256
    : public testing::Test
{
public:
    const std::array<uint8_t, 64> ecDsa256PublicKey = { { 0x3e, 0x6b, 0xe0, 0x8c, 0x64, 0x3e, 0xf8, 0x26,
        0x3f, 0x9c, 0xdc, 0x7e, 0x75, 0x5d, 0x17, 0x04,
        0xf2, 0xfd, 0xe0, 0xf4, 0x0f, 0x35, 0xc3, 0x01,
        0x9d, 0x8b, 0x2b, 0xb1, 0xd2, 0x87, 0xdb, 0x5e,
        0x92, 0x9e, 0x87, 0x66, 0x59, 0x44, 0xb5, 0xb7,
        0xa5, 0xab, 0x7f, 0xbf, 0xe7, 0x8c, 0x8f, 0xee,
        0x9d, 0xa2, 0x67, 0xf2, 0x7a, 0x27, 0xb4, 0x56,
        0x92, 0x11, 0x90, 0x2e, 0x54, 0x64, 0x31, 0x4c } };

    application::VerifierEcDsa256 verifier{ infra::MakeConstByteRange(ecDsa256PublicKey) };
};

TEST_F(VerifierEcDsaTest256, incorrect_hash_size_is_invalid)
{
    hal::SynchronousFlashStub flash(1, 64);
    EXPECT_EQ(false, verifier.IsValid(flash, { 0, 0 }, { 0, 4 }));
}

TEST_F(VerifierEcDsaTest256, incorrect_hash_is_invalid)
{
    hal::SynchronousFlashStub flash(1, 64);

    flash.sectors = {
        { 0, 1, 2, 3 }
    };

    flash.sectors[0].insert(flash.sectors[0].end(), 56, 0xFF);

    EXPECT_EQ(false, verifier.IsValid(flash, { 4, 60 }, { 0, 4 }));
}

TEST_F(VerifierEcDsaTest256, correct_hash_as_signature_is_valid)
{
    hal::SynchronousFlashStub flash(1, 128);

    flash.sectors = {
        { 0, 1, 2, 3,
            0x07, 0x81, 0x0e, 0xa9, 0x74, 0xce, 0xa5, 0x77,
            0x3e, 0x63, 0xb8, 0x97, 0xf3, 0x7e, 0x3b, 0xe9,
            0xa0, 0x9e, 0x7a, 0x5f, 0xe9, 0xb9, 0x71, 0xa4,
            0x4d, 0x10, 0x65, 0xac, 0x2a, 0x3a, 0x93, 0x11,
            0x94, 0xdb, 0x42, 0x97, 0xe8, 0x53, 0xc9, 0x8e,
            0xe2, 0x8b, 0xd6, 0xb0, 0x43, 0x25, 0xd7, 0xc9,
            0x32, 0x66, 0x12, 0x3a, 0xac, 0x6d, 0x7d, 0x0e,
            0x5c, 0x3b, 0x79, 0x2d, 0xde, 0x78, 0xb1, 0xd5 }
    };

    EXPECT_EQ(true, verifier.IsValid(flash, { 4, 68 }, { 0, 4 }));
}
