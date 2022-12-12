#include "hal/synchronous_interfaces/test_doubles/SynchronousFlashStub.hpp"
#include "upgrade/boot_loader/VerifierHashOnly.hpp"
#include "gmock/gmock.h"

TEST(VerifierHashOnlyTest, incorrect_hash_size_is_invalid)
{
    hal::SynchronousFlashStub flash(1, 64);
    application::VerifierHashOnly verifier;
    EXPECT_EQ(false, verifier.IsValid(flash, { 0, 0 }, { 0, 4 }));
}

TEST(VerifierHashOnlyTest, incorrect_hash_is_invalid)
{
    hal::SynchronousFlashStub flash(1, 64);

    flash.sectors = {
        { 0, 1, 2, 3 }
    };

    flash.sectors[0].insert(flash.sectors[0].end(), 32, 0xFF);

    application::VerifierHashOnly verifier;
    EXPECT_EQ(false, verifier.IsValid(flash, { 4, 36 }, { 0, 4 }));
}

TEST(VerifierHashOnlyTest, correct_hash_as_signature_is_valid)
{
    hal::SynchronousFlashStub flash(1, 64);

    flash.sectors = {
        { 0, 1, 2, 3,
            0x05, 0x4E, 0xDE, 0xC1, 0xD0, 0x21, 0x1F, 0x62,
            0x4F, 0xED, 0x0C, 0xBC, 0xA9, 0xD4, 0xF9, 0x40,
            0x0B, 0x0E, 0x49, 0x1C, 0x43, 0x74, 0x2A, 0xF2,
            0xC5, 0xB0, 0xAB, 0xEB, 0xF0, 0xC9, 0x90, 0xD8 }
    };

    application::VerifierHashOnly verifier;
    EXPECT_EQ(true, verifier.IsValid(flash, { 4, 36 }, { 0, 4 }));
}
