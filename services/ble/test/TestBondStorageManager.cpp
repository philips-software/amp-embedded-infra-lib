#include "services/ble/BondStorageManager.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "gmock/gmock.h"

class BondStorageMock
    : public services::BondStorage
{
public:
    MOCK_METHOD1(BondStorageManagerCreated, void(services::BondStorageManager& manager));
    MOCK_METHOD1(UpdateBondedDevice, void(hal::MacAddress address));
    MOCK_METHOD1(RemoveBond, void(hal::MacAddress address));
    MOCK_METHOD0(RemoveAllBonds, void());
    MOCK_METHOD1(RemoveBondIf, void(infra::Function<bool(hal::MacAddress)> onAddress));
    MOCK_CONST_METHOD0(GetMaxNumberOfBonds, uint32_t());
    MOCK_METHOD1(IsBondStored, bool(hal::MacAddress address));
    MOCK_METHOD1(IterateBondedDevices, void(infra::Function<void(hal::MacAddress)> onAddress));
};

class BondStorageManagerTest
    : public testing::Test
{
public:
    void ExpectBondStorageManagerCreated()
    {
        EXPECT_CALL(referenceStorage, BondStorageManagerCreated(testing::_));
        EXPECT_CALL(otherStorage, BondStorageManagerCreated(testing::_));
    }

    void ExpectGetMaxNumberOfBonds()
    {
        EXPECT_CALL(referenceStorage, GetMaxNumberOfBonds()).WillOnce(testing::Return(maxNumberOfBonds));
        EXPECT_CALL(otherStorage, GetMaxNumberOfBonds()).WillOnce(testing::Return(maxNumberOfBonds));
    }

    void ExpectSyncBondStoragesEqualList()
    {
        infra::BoundedVector<hal::MacAddress>::WithMaxSize<2> list({address1, address2});

        EXPECT_CALL(otherStorage, RemoveBondIf(testing::_)).WillOnce([this, list](infra::Function<bool(hal::MacAddress)> onAddress)
        {
            for(auto address : list)
            {
                EXPECT_CALL(referenceStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(address)))).WillOnce(testing::Return(true));
                onAddress(address);
            }
        });

        EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_)).WillOnce([this, list](infra::Function<void(hal::MacAddress)> onAddress)
        {
            for(auto address : list)
            {
                EXPECT_CALL(otherStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(address)))).WillOnce(testing::Return(true));
                onAddress(address);
            }
        });
    }

    hal::MacAddress address1{{0x00, 0x01, 0x02, 0x03, 0x04, 0x05}};
    hal::MacAddress address2{{0x06, 0x07, 0x08, 0x09, 0x10, 0x11}};
    hal::MacAddress address3{{0x12, 0x13, 0x14, 0x15, 0x16, 0x17}};

    testing::StrictMock<BondStorageMock> referenceStorage;
    testing::StrictMock<BondStorageMock> otherStorage;
    uint32_t maxNumberOfBonds = 3;
};

TEST_F(BondStorageManagerTest, construction_notifies_bondstorages_of_creation_checks_max_number_of_bonds_and_synchronises_the_storages)
{
    ExpectBondStorageManagerCreated();
    ExpectGetMaxNumberOfBonds();
    ExpectSyncBondStoragesEqualList();
    services::BondStorageManagerImpl bondStorageManager(referenceStorage, otherStorage);
}

TEST_F(BondStorageManagerTest, construction_synchronises_empty_bond_storages)
{
    ExpectBondStorageManagerCreated();
    ExpectGetMaxNumberOfBonds();

    EXPECT_CALL(otherStorage, RemoveBondIf(testing::_));
    EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_));

    services::BondStorageManagerImpl bondStorageManager(referenceStorage, otherStorage);
}

TEST_F(BondStorageManagerTest, construction_synchronises_empty_reference_bond_storages)
{
    ExpectBondStorageManagerCreated();
    ExpectGetMaxNumberOfBonds();

    std::array<hal::MacAddress, 2> otherList = {address1, address2};

    EXPECT_CALL(otherStorage, RemoveBondIf(testing::_)).WillOnce([this, otherList](infra::Function<bool(hal::MacAddress)> onAddress)
    {
        auto shouldRemove = false;

        EXPECT_CALL(referenceStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(otherList.front())))).WillOnce(testing::Return(false));
        shouldRemove = onAddress(otherList.front());
        EXPECT_EQ(shouldRemove, true);

        EXPECT_CALL(referenceStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(otherList.back())))).WillOnce(testing::Return(false));
        shouldRemove = onAddress(otherList.back());
        EXPECT_EQ(shouldRemove, true);
    });

    EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_));

    services::BondStorageManagerImpl bondStorageManager(referenceStorage, otherStorage);
}

TEST_F(BondStorageManagerTest, construction_synchronises_empty_other_bond_storages)
{
    ExpectBondStorageManagerCreated();
    ExpectGetMaxNumberOfBonds();

    std::array<hal::MacAddress, 2> referenceList = {address1, address2};

    EXPECT_CALL(otherStorage, RemoveBondIf(testing::_));

    EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_)).WillOnce([this, referenceList](infra::Function<void(hal::MacAddress)> onAddress)
    {
        EXPECT_CALL(otherStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(referenceList.front())))).WillOnce(testing::Return(false));
        EXPECT_CALL(otherStorage, UpdateBondedDevice(infra::CheckByteRangeContents(infra::MakeRange(referenceList.front()))));
        onAddress(referenceList.front());

        EXPECT_CALL(otherStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(referenceList.back())))).WillOnce(testing::Return(false));
        EXPECT_CALL(otherStorage, UpdateBondedDevice(infra::CheckByteRangeContents(infra::MakeRange(referenceList.back()))));
        onAddress(referenceList.back());
    });

    services::BondStorageManagerImpl bondStorageManager(referenceStorage, otherStorage);
}

TEST_F(BondStorageManagerTest, construction_synchronises_unequal_bond_storages)
{
    ExpectBondStorageManagerCreated();
    ExpectGetMaxNumberOfBonds();

    std::array<hal::MacAddress, 2> referenceList = {address1, address2};
    std::array<hal::MacAddress, 2> otherList = {address1, address3};

    EXPECT_CALL(otherStorage, RemoveBondIf(testing::_)).WillOnce([this, otherList](infra::Function<bool(hal::MacAddress)> onAddress)
    {
        auto shouldRemove = false;
        EXPECT_CALL(referenceStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(otherList.front())))).WillOnce(testing::Return(true));
        shouldRemove = onAddress(otherList.front());
        EXPECT_EQ(shouldRemove, false);

        EXPECT_CALL(referenceStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(otherList.back())))).WillOnce(testing::Return(false));
        shouldRemove = onAddress(otherList.back());
        EXPECT_EQ(shouldRemove, true);
    });

    EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_)).WillOnce([this, referenceList](infra::Function<void(hal::MacAddress)> onAddress)
    {
        EXPECT_CALL(otherStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(referenceList.front())))).WillOnce(testing::Return(true));
        onAddress(referenceList.front());

        EXPECT_CALL(otherStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(referenceList.back())))).WillOnce(testing::Return(false));
        EXPECT_CALL(otherStorage, UpdateBondedDevice(infra::CheckByteRangeContents(infra::MakeRange(referenceList.back()))));
        onAddress(referenceList.back());
    });

    services::BondStorageManagerImpl bondStorageManager(referenceStorage, otherStorage);
}

TEST_F(BondStorageManagerTest, bonded_device_connected_is_forwarded_to_bondstorages)
{
    ExpectBondStorageManagerCreated();
    ExpectGetMaxNumberOfBonds();
    ExpectSyncBondStoragesEqualList();
    services::BondStorageManagerImpl bondStorageManager(referenceStorage, otherStorage);

    EXPECT_CALL(referenceStorage, UpdateBondedDevice(infra::CheckByteRangeContents(infra::MakeRange(address1))));
    EXPECT_CALL(otherStorage, UpdateBondedDevice(infra::CheckByteRangeContents(infra::MakeRange(address1))));
    bondStorageManager.UpdateBondedDevice(address1);
}

TEST_F(BondStorageManagerTest, remove_bond_is_forwarded_to_bondstorages)
{
    ExpectBondStorageManagerCreated();
    ExpectGetMaxNumberOfBonds();
    ExpectSyncBondStoragesEqualList();
    services::BondStorageManagerImpl bondStorageManager(referenceStorage, otherStorage);

    EXPECT_CALL(referenceStorage, RemoveBond(infra::CheckByteRangeContents(infra::MakeRange(address1))));
    EXPECT_CALL(otherStorage, RemoveBond(infra::CheckByteRangeContents(infra::MakeRange(address1))));
    bondStorageManager.RemoveBond(address1);
}

TEST_F(BondStorageManagerTest, remove_all_bonds_is_forwarded_to_bondstorages)
{
    ExpectBondStorageManagerCreated();
    ExpectGetMaxNumberOfBonds();
    ExpectSyncBondStoragesEqualList();
    services::BondStorageManagerImpl bondStorageManager(referenceStorage, otherStorage);

    EXPECT_CALL(referenceStorage, RemoveAllBonds());
    EXPECT_CALL(otherStorage, RemoveAllBonds());
    bondStorageManager.RemoveAllBonds();
}