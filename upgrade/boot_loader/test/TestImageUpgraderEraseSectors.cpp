#include "hal/synchronous_interfaces/test_doubles/SynchronousFlashStub.hpp"
#include "upgrade/boot_loader/DecryptorNone.hpp"
#include "upgrade/boot_loader/ImageUpgraderEraseSectors.hpp"
#include "gmock/gmock.h"

class ImageUpgraderEraseSectorsTest
    : public testing::Test
{
public:
    ImageUpgraderEraseSectorsTest()
        : internalFlash(3, 1)
        , upgradePackFlash(1, 512)
    {}

public:
    application::DecryptorNone decryptor;
    hal::SynchronousFlashStub internalFlash;
    hal::SynchronousFlashStub upgradePackFlash;
};

TEST_F(ImageUpgraderEraseSectorsTest, upgrade_erases_sectors)
{
    internalFlash.sectors = { { 1 }, { 2 }, { 3 } };

    application::ImageUpgraderEraseSectors upgrader("upgrader", decryptor, internalFlash, 1, 2);
    upgrader.Upgrade(upgradePackFlash, 0, 4, 0);

    EXPECT_EQ((std::vector<std::vector<uint8_t>>{ { 1 }, { 0xff }, { 3 } }), internalFlash.sectors);
}
