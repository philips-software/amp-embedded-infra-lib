#include "services/ble/PersistingBondStorage.hpp"
#include "services/util/test_doubles/ConfigurationStoreMock.hpp"
#include "gmock/gmock.h"

class PersistingBondStorageTest
    : public testing::Test
{
public:
    void CompareStorages()
    {
        EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(flashStorage), infra::MakeRange(ramStorage)));
    }

public:
    testing::StrictMock<services::ConfigurationStoreInterfaceMock> configurationStore;
    infra::BoundedVector<uint8_t>::WithMaxSize<3> flashStorage{{ 1, 2, 3 }};
    std::array<uint8_t, 3> ramStorage{{ 4, 5, 6 }};

    services::PersistingBondStorage persistingBondStorage{{configurationStore, flashStorage}, infra::MakeByteRange(ramStorage)};
};

TEST_F(PersistingBondStorageTest, construct_updates_ram_with_flash_storage)
{
    CompareStorages();
}

TEST_F(PersistingBondStorageTest, update_updates_flash_with_ram_storage)
{
    ramStorage = {7, 8, 9};

    EXPECT_CALL(configurationStore, Write());
    persistingBondStorage.Update();

    CompareStorages();
}
