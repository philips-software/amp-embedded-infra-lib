#include "hal/synchronous_interfaces/test_doubles/SynchronousFlashStub.hpp"
#include "upgrade/boot_loader/DecryptorNone.hpp"
#include "upgrade/boot_loader/ImageUpgraderFlash.hpp"
#include "gmock/gmock.h"

class ImageUpgraderFlashTest
    : public testing::Test
{
public:
    ImageUpgraderFlashTest()
        : internalFlash(1, 512)
        , upgradePackFlash(1, 512)
    {}

public:
    application::DecryptorNone decryptor;
    hal::SynchronousFlashStub internalFlash;
    hal::SynchronousFlashStub upgradePackFlash;
};

TEST_F(ImageUpgraderFlashTest, Construction)
{
    application::ImageUpgraderFlash::WithBlockSize<256> upgrader("upgrader", decryptor, internalFlash, 0);
}

TEST_F(ImageUpgraderFlashTest, UpgradeCopiesFlash)
{
    upgradePackFlash.sectors[0] = { 1, 2, 3, 4 };
    internalFlash.sectors[0].resize(upgradePackFlash.sectors[0].size());

    application::ImageUpgraderFlash::WithBlockSize<256> upgrader("upgrader", decryptor, internalFlash, 0);
    upgrader.Upgrade(upgradePackFlash, 0, 4, 0);

    EXPECT_EQ((std::vector<uint8_t>{ 1, 2, 3, 4 }), internalFlash.sectors[0]);
}

TEST_F(ImageUpgraderFlashTest, UpgradeCopiesFlashToDestinationAddress)
{
    upgradePackFlash.sectors[0] = { 1, 2, 3, 4 };
    internalFlash.sectors[0].resize(upgradePackFlash.sectors[0].size() + 4);

    application::ImageUpgraderFlash::WithBlockSize<256> upgrader("upgrader", decryptor, internalFlash, 0);
    upgrader.Upgrade(upgradePackFlash, 0, 4, 4);

    internalFlash.sectors[0].erase(internalFlash.sectors[0].begin(), internalFlash.sectors[0].begin() + 4);
    EXPECT_EQ((std::vector<uint8_t>{ 1, 2, 3, 4 }), internalFlash.sectors[0]);
}

TEST_F(ImageUpgraderFlashTest, UpgradeCopiesFlashStartingFromOffset)
{
    upgradePackFlash.sectors[0] = { 1, 2, 3, 4 };
    internalFlash.sectors[0].resize(upgradePackFlash.sectors[0].size());

    application::ImageUpgraderFlash::WithBlockSize<256> upgrader("upgrader", decryptor, internalFlash, 0);
    upgrader.Upgrade(upgradePackFlash, 2, 2, 0);

    EXPECT_EQ((std::vector<uint8_t>{ 3, 4, 0xff, 0xff }), internalFlash.sectors[0]);
}

TEST_F(ImageUpgraderFlashTest, UpgradeUsesDestinationAddressOffset)
{
    upgradePackFlash.sectors[0] = { 1, 2, 3, 4 };
    internalFlash.sectors[0].resize(upgradePackFlash.sectors[0].size());

    application::ImageUpgraderFlash::WithBlockSize<256> upgrader("upgrader", decryptor, internalFlash, -4);
    upgrader.Upgrade(upgradePackFlash, 0, 4, 4);

    EXPECT_EQ((std::vector<uint8_t>{ 1, 2, 3, 4 }), internalFlash.sectors[0]);
}

TEST_F(ImageUpgraderFlashTest, SectorsAreFirstErased)
{
    upgradePackFlash.sectors[0] = { 1, 2, 3, 4 };
    internalFlash.sectors[0].clear();
    internalFlash.sectors[0].resize(upgradePackFlash.sectors[0].size());

    application::ImageUpgraderFlash::WithBlockSize<256> upgrader("upgrader", decryptor, internalFlash, 0);
    upgrader.Upgrade(upgradePackFlash, 0, 4, 0);

    EXPECT_EQ((std::vector<uint8_t>{ 1, 2, 3, 4 }), internalFlash.sectors[0]);
}
