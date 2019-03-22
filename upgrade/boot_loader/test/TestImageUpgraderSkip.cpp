#include "gmock/gmock.h"
#include "hal/synchronous_interfaces/test_doubles/SynchronousFlashStub.hpp"
#include "upgrade/boot_loader/ImageUpgraderSkip.hpp"

TEST(ImageUpgraderSkipTest, ConstructionWithTargetName)
{
    application::ImageUpgraderSkip upgrader("skip");
    EXPECT_STREQ("skip", upgrader.TargetName());
}

TEST(ImageUpgraderSkipTest, DecryptorHasNoState)
{
    application::ImageUpgraderSkip upgrader("skip");
    EXPECT_TRUE(upgrader.ImageDecryptor().StateBuffer().empty());
}

TEST(ImageUpgraderSkipTest, DecryptorDoesntModify)
{
    std::array<uint8_t, 4> data = { 0, 1, 2, 3 };
    std::array<uint8_t, 4> originalData = data;
    application::ImageUpgraderSkip upgrader("skip");
    upgrader.ImageDecryptor().DecryptPart(data);
    EXPECT_EQ(originalData, data);
}

TEST(ImageUpgraderSkipTest, DecryptorAuthenticateReturnsTrue)
{
    application::ImageUpgraderSkip upgrader("skip");
    EXPECT_TRUE(upgrader.ImageDecryptor().DecryptAndAuthenticate(infra::ByteRange()));
}

TEST(ImageUpgraderSkipTest, UpgradeReturnsNoError)
{
    hal::SynchronousFlashStub flash(1, 1);
    application::ImageUpgraderSkip upgrader("skip");
    EXPECT_EQ(0, upgrader.Upgrade(flash, 0, 0, 0));
}
