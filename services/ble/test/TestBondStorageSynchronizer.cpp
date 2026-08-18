#include "infra/util/Function.hpp"
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

    static services::Bond MakeBond(hal::MacAddress address, infra::BoundedConstString deviceName)
    {
        return services::Bond{ services::GapAddress{ address, services::GapDeviceAddressType::publicAddress }, deviceName, services::Role::peripheral };
    }

    hal::MacAddress address1{ { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 } };
    hal::MacAddress address2{ { 0x06, 0x07, 0x08, 0x09, 0x10, 0x11 } };
    hal::MacAddress address3{ { 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 } };

    services::Bond bond1{ MakeBond(address1, "device1") };
    services::Bond bond2{ MakeBond(address2, "device2") };

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

TEST_F(BondStorageSynchronizerTest, construction_removes_all_bonds_from_other_storage_when_reference_storage_is_empty)
{
    EXPECT_CALL(otherStorage, RemoveBondIf(testing::_)).WillOnce([this](const infra::Function<bool(hal::MacAddress)>& onAddress)
        {
            EXPECT_CALL(referenceStorage, IsBondStored(address1)).WillOnce(testing::Return(false));
            ASSERT_THAT(onAddress(address1), testing::IsTrue());

            EXPECT_CALL(referenceStorage, IsBondStored(address2)).WillOnce(testing::Return(false));
            ASSERT_THAT(onAddress(address2), testing::IsTrue());
        });

    EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_));

    services::BondStorageSynchronizerImpl bondStorageSynchronizer(referenceStorage, otherStorage);
}

TEST_F(BondStorageSynchronizerTest, construction_adds_all_bonds_to_other_storage_when_other_storage_is_empty)
{
    EXPECT_CALL(otherStorage, RemoveBondIf(testing::_));

    EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_)).WillOnce([this](const infra::Function<void(const services::Bond&)>& onBond)
        {
            EXPECT_CALL(otherStorage, IsBondStored(address1)).WillOnce(testing::Return(false));
            EXPECT_CALL(otherStorage, AddBond(bond1));
            onBond(bond1);

            EXPECT_CALL(otherStorage, IsBondStored(address2)).WillOnce(testing::Return(false));
            EXPECT_CALL(otherStorage, AddBond(bond2));
            onBond(bond2);
        });

    services::BondStorageSynchronizerImpl bondStorageSynchronizer(referenceStorage, otherStorage);
}

TEST_F(BondStorageSynchronizerTest, construction_synchronises_unequal_bond_storages)
{
    EXPECT_CALL(otherStorage, RemoveBondIf(testing::_)).WillOnce([this](const infra::Function<bool(hal::MacAddress)>& onAddress)
        {
            EXPECT_CALL(referenceStorage, IsBondStored(address1)).WillOnce(testing::Return(true));
            ASSERT_THAT(onAddress(address1), testing::IsFalse());

            EXPECT_CALL(referenceStorage, IsBondStored(address3)).WillOnce(testing::Return(false));
            ASSERT_THAT(onAddress(address3), testing::IsTrue());
        });

    EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_)).WillOnce([this](const infra::Function<void(const services::Bond&)>& onBond)
        {
            EXPECT_CALL(otherStorage, IsBondStored(address1)).WillOnce(testing::Return(true));
            onBond(bond1);

            EXPECT_CALL(otherStorage, IsBondStored(address2)).WillOnce(testing::Return(false));
            EXPECT_CALL(otherStorage, AddBond(bond2));
            onBond(bond2);
        });

    services::BondStorageSynchronizerImpl bondStorageSynchronizer(referenceStorage, otherStorage);
}

class BondStorageSynchronizerTestWithConstruction
    : public BondStorageSynchronizerTest
{
public:
    void ExpectSyncBondStoragesWithEqualStorages()
    {
        EXPECT_CALL(otherStorage, RemoveBondIf(testing::_)).WillOnce([this](const infra::Function<bool(hal::MacAddress)>& onAddress)
            {
                EXPECT_CALL(referenceStorage, IsBondStored(address1)).WillOnce(testing::Return(true));
                onAddress(address1);

                EXPECT_CALL(referenceStorage, IsBondStored(address2)).WillOnce(testing::Return(true));
                onAddress(address2);
            });

        EXPECT_CALL(referenceStorage, IterateBondedDevices(testing::_)).WillOnce([this](const infra::Function<void(const services::Bond&)>& onBond)
            {
                EXPECT_CALL(otherStorage, IsBondStored(address1)).WillOnce(testing::Return(true));
                onBond(bond1);

                EXPECT_CALL(otherStorage, IsBondStored(address2)).WillOnce(testing::Return(true));
                onBond(bond2);
            });
    }

    infra::Execute execute{ [this]()
        {
            ExpectSyncBondStoragesWithEqualStorages();
        } };
    services::BondStorageSynchronizerImpl bondStorageSynchronizer{ referenceStorage, otherStorage };
};

TEST_F(BondStorageSynchronizerTestWithConstruction, construction_notifies_bondstorages_of_creation_checks_max_number_of_bonds_and_synchronises_the_storages)
{
    EXPECT_THAT(bondStorageSynchronizer.GetMaxNumberOfBonds(), testing::Eq(maxNumberOfBonds));
}

TEST_F(BondStorageSynchronizerTestWithConstruction, add_bond_is_forwarded_to_bondstorages)
{
    EXPECT_CALL(referenceStorage, AddBond(bond1));
    EXPECT_CALL(otherStorage, AddBond(bond1));
    bondStorageSynchronizer.AddBond(bond1);
}

TEST_F(BondStorageSynchronizerTestWithConstruction, mark_as_recently_used_is_forwarded_to_bondstorages)
{
    EXPECT_CALL(referenceStorage, MarkAsRecentlyUsed(address1));
    EXPECT_CALL(otherStorage, MarkAsRecentlyUsed(address1));
    bondStorageSynchronizer.MarkAsRecentlyUsed(address1);
}

TEST_F(BondStorageSynchronizerTestWithConstruction, remove_bond_is_forwarded_to_bondstorages)
{
    EXPECT_CALL(referenceStorage, RemoveBond(address1));
    EXPECT_CALL(otherStorage, RemoveBond(address1));
    bondStorageSynchronizer.RemoveBond(address1);
}

TEST_F(BondStorageSynchronizerTestWithConstruction, remove_all_bonds_is_forwarded_to_bondstorages)
{
    EXPECT_CALL(referenceStorage, RemoveAllBonds());
    EXPECT_CALL(otherStorage, RemoveAllBonds());
    bondStorageSynchronizer.RemoveAllBonds();
}
