#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/ble/BondStorageSynchronizer.hpp"
#include "services/ble/test_doubles/BondStorageSynchronizerMock.hpp"
#include "gmock/gmock.h"

class BondStorageSynchronizerTest
    : public testing::Test
{
public:
    void ExpectBondStorageSynchronizerCreated()
    {
        EXPECT_CALL(referenceStorage, BondStorageSynchronizerCreated(testing::_));
        EXPECT_CALL(otherStorage, BondStorageSynchronizerCreated(testing::_));
    }

    void ExpectGetMaxNumberOfBonds()
    {
        EXPECT_CALL(referenceStorage, GetMaxNumberOfBonds()).WillOnce(testing::Return(maxNumberOfBonds));
        EXPECT_CALL(otherStorage, GetMaxNumberOfBonds()).WillOnce(testing::Return(maxNumberOfBonds));
    }

    hal::MacAddress address1{ { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 } };
    hal::MacAddress address2{ { 0x06, 0x07, 0x08, 0x09, 0x10, 0x11 } };
    hal::MacAddress address3{ { 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 } };

    testing::StrictMock<services::BondStorageMock> referenceStorage;
    testing::StrictMock<services::BondStorageMock> otherStorage;
    uint32_t maxNumberOfBonds = 3;

    infra::Execute execute{ [this]()
        {
            ExpectBondStorageSynchronizerCreated();
            ExpectGetMaxNumberOfBonds();
        } };
};

TEST_F(BondStorageSynchronizerTest, construction_synchronises_empty_bond_storages)
{
    EXPECT_CALL(otherStorage, RemoveBondIf(testing::_));
    EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_));

    services::BondStorageSynchronizerImpl bondStorageSynchronizer(referenceStorage, otherStorage);
}

TEST_F(BondStorageSynchronizerTest, construction_synchronises_empty_reference_bond_storages)
{
    std::array<hal::MacAddress, 2> otherList = { address1, address2 };

    EXPECT_CALL(otherStorage, RemoveBondIf(testing::_)).WillOnce([this, otherList](infra::Function<bool(hal::MacAddress)> onAddress)
        {
            EXPECT_CALL(referenceStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(otherList.front())))).WillOnce(testing::Return(false));
            ASSERT_TRUE(onAddress(otherList.front()));

            EXPECT_CALL(referenceStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(otherList.back())))).WillOnce(testing::Return(false));
            ASSERT_TRUE(onAddress(otherList.back()));
        });

    EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_));

    services::BondStorageSynchronizerImpl bondStorageSynchronizer(referenceStorage, otherStorage);
}

TEST_F(BondStorageSynchronizerTest, construction_synchronises_empty_other_bond_storages)
{
    std::array<hal::MacAddress, 2> referenceList = { address1, address2 };

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

    services::BondStorageSynchronizerImpl bondStorageSynchronizer(referenceStorage, otherStorage);
}

TEST_F(BondStorageSynchronizerTest, construction_synchronises_unequal_bond_storages)
{
    std::array<hal::MacAddress, 2> referenceList = { address1, address2 };
    std::array<hal::MacAddress, 2> otherList = { address1, address3 };

    EXPECT_CALL(otherStorage, RemoveBondIf(testing::_)).WillOnce([this, otherList](infra::Function<bool(hal::MacAddress)> onAddress)
        {
            EXPECT_CALL(referenceStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(otherList.front())))).WillOnce(testing::Return(true));
            ASSERT_FALSE(onAddress(otherList.front()));

            EXPECT_CALL(referenceStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(otherList.back())))).WillOnce(testing::Return(false));
            ASSERT_TRUE(onAddress(otherList.back()));
        });

    EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_)).WillOnce([this, referenceList](infra::Function<void(hal::MacAddress)> onAddress)
        {
            EXPECT_CALL(otherStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(referenceList.front())))).WillOnce(testing::Return(true));
            onAddress(referenceList.front());

            EXPECT_CALL(otherStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(referenceList.back())))).WillOnce(testing::Return(false));
            EXPECT_CALL(otherStorage, UpdateBondedDevice(infra::CheckByteRangeContents(infra::MakeRange(referenceList.back()))));
            onAddress(referenceList.back());
        });

    services::BondStorageSynchronizerImpl bondStorageSynchronizer(referenceStorage, otherStorage);
}

class BondStorageSynchronizerTestWithConstruction
    : public BondStorageSynchronizerTest
{
public:
    void ExpectSyncBondStoragesWithEqualList()
    {
        std::array<hal::MacAddress, 2> list({ address1, address2 });

        EXPECT_CALL(otherStorage, RemoveBondIf(testing::_)).WillOnce([this, list](infra::Function<bool(hal::MacAddress)> onAddress)
            {
                for (auto address : list)
                {
                    EXPECT_CALL(referenceStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(address)))).WillOnce(testing::Return(true));
                    onAddress(address);
                }
            });

        EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_)).WillOnce([this, list](infra::Function<void(hal::MacAddress)> onAddress)
            {
                for (auto address : list)
                {
                    EXPECT_CALL(otherStorage, IsBondStored(infra::CheckByteRangeContents(infra::MakeRange(address)))).WillOnce(testing::Return(true));
                    onAddress(address);
                }
            });
    }

    infra::Execute execute{ [this]()
        {
            ExpectSyncBondStoragesWithEqualList();
        } };
    services::BondStorageSynchronizerImpl bondStorageSynchronizer{ referenceStorage, otherStorage };
};

TEST_F(BondStorageSynchronizerTestWithConstruction, construction_notifies_bondstorages_of_creation_checks_max_number_of_bonds_and_synchronises_the_storages)
{}

TEST_F(BondStorageSynchronizerTestWithConstruction, bonded_device_connected_is_forwarded_to_bondstorages)
{
    EXPECT_CALL(referenceStorage, UpdateBondedDevice(infra::CheckByteRangeContents(infra::MakeRange(address1))));
    EXPECT_CALL(otherStorage, UpdateBondedDevice(infra::CheckByteRangeContents(infra::MakeRange(address1))));
    bondStorageSynchronizer.UpdateBondedDevice(address1);
}

TEST_F(BondStorageSynchronizerTestWithConstruction, remove_bond_is_forwarded_to_bondstorages)
{
    EXPECT_CALL(referenceStorage, RemoveBond(infra::CheckByteRangeContents(infra::MakeRange(address1))));
    EXPECT_CALL(otherStorage, RemoveBond(infra::CheckByteRangeContents(infra::MakeRange(address1))));
    bondStorageSynchronizer.RemoveBond(address1);
}

TEST_F(BondStorageSynchronizerTestWithConstruction, remove_all_bonds_is_forwarded_to_bondstorages)
{
    EXPECT_CALL(referenceStorage, RemoveAllBonds());
    EXPECT_CALL(otherStorage, RemoveAllBonds());
    bondStorageSynchronizer.RemoveAllBonds();
}
