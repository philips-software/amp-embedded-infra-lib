#include "gmock/gmock.h"
#include "services/ble/BondBlobPersistence.hpp"
#include "services/util/test_doubles/ConfigurationStoreMock.hpp"

class BondBlobPersistenceTest
    : public testing::Test
{
public:
    void Construct()
    {
        bondBlobPersistence.Emplace(flashStorageAccess, infra::MakeByteRange(ramStorage));
    }

    void FillStorages(std::vector<uint8_t> flashData, std::array<uint8_t, 3> ramData)
    {
        flashStorage.assign(infra::MakeRange(flashData));
        ramStorage = ramData;
    }

    void FillStoragesAndConstruct(std::vector<uint8_t> flashData, std::array<uint8_t, 3> ramData)
    {
        FillStorages(flashData, ramData);
        Construct();
    }

    void ExpectStoragesEqual(std::vector<uint8_t> data)
    {
        EXPECT_TRUE(infra::ContentsEqual(flashStorage.range(), infra::MakeRange(data)));
        EXPECT_TRUE(infra::ContentsEqual(flashStorage.range(), infra::MakeRange(ramStorage)));
    }

public:
    testing::StrictMock<services::ConfigurationStoreInterfaceMock> configurationStore;
    infra::BoundedVector<uint8_t>::WithMaxSize<3> flashStorage;
    services::ConfigurationStoreAccess<infra::BoundedVector<uint8_t>> flashStorageAccess{configurationStore, flashStorage};
    std::array<uint8_t, 3> ramStorage{ 0, 0, 0 };

    infra::Optional<services::BondBlobPersistence> bondBlobPersistence;
};

TEST_F(BondBlobPersistenceTest, construct_updates_ram_with_empty_flash_storage)
{
    Construct();
    ExpectStoragesEqual({ 0, 0, 0 });
}

TEST_F(BondBlobPersistenceTest, construct_updates_ram_with_flash_storage)
{
    FillStoragesAndConstruct({ 1, 2, 3 }, { 4, 5, 6 });
    ExpectStoragesEqual({ 1, 2, 3 });
}

TEST_F(BondBlobPersistenceTest, update_updates_flash_with_ram_storage)
{
    FillStoragesAndConstruct({ 1, 2, 3 }, { 4, 5, 6 });

    ramStorage = { 7, 8, 9 };

    EXPECT_CALL(configurationStore, Write());
    bondBlobPersistence->Update();

    ExpectStoragesEqual({ 7, 8, 9 });
}
