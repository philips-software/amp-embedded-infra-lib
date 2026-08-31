#include "infra/util/Function.hpp"
#include "services/ble/BondStorageSynchronizer.hpp"
#include "services/ble/test_doubles/BondStorageSynchronizerMock.hpp"
#include "gmock/gmock.h"
#include <optional>

class BondStorageSynchronizerTest
    : public testing::Test
{
public:
    void ExpectBondStorageSynchronizerCreated()
    {
        EXPECT_CALL(bondStorage, BondStorageSynchronizerCreated(testing::_));
        EXPECT_CALL(absoluteStorage, BondStorageSynchronizerCreated(testing::_));
    }

    void ExpectGetMaxNumberOfBonds()
    {
        EXPECT_CALL(absoluteStorage, GetMaxNumberOfBonds()).Times(2).WillRepeatedly(testing::Return(maxNumberOfBonds));
        EXPECT_CALL(bondStorage, GetMaxNumberOfBonds()).Times(2).WillRepeatedly(testing::Return(maxNumberOfBonds));
    }

    static services::GapAddress MakeGapAddress(hal::MacAddress address)
    {
        return services::GapAddress{ address, services::GapDeviceAddressType::publicAddress };
    }

    static services::Bond MakeBond(const services::GapAddress& address, infra::BoundedConstString deviceName)
    {
        return services::Bond{ address, deviceName };
    }

    services::GapAddress gapAddress1{ MakeGapAddress({ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 }) };
    services::GapAddress gapAddress2{ MakeGapAddress({ 0x06, 0x07, 0x08, 0x09, 0x10, 0x11 }) };
    services::GapAddress gapAddress3{ MakeGapAddress({ 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 }) };

    services::Bond bond1{ MakeBond(gapAddress1, "device1") };
    services::Bond bond2{ MakeBond(gapAddress2, "device2") };

    testing::StrictMock<services::BondStorageAbsoluteMock> absoluteStorage;
    testing::StrictMock<services::BondStorageMock> bondStorage;
    uint32_t maxNumberOfBonds = 3;

    infra::Execute execute{ [this]()
        {
            ExpectBondStorageSynchronizerCreated();
            ExpectGetMaxNumberOfBonds();
        } };
};

TEST_F(BondStorageSynchronizerTest, construction_synchronises_empty_bond_storages)
{
    EXPECT_CALL(bondStorage, RemoveBondIf(testing::_));
    EXPECT_CALL(absoluteStorage, IterateBondedDevices(testing::_));

    services::BondStorageSynchronizerImpl bondStorageSynchronizer(absoluteStorage, bondStorage);
}

TEST_F(BondStorageSynchronizerTest, construction_removes_bond_from_bond_storage_when_not_in_absolute_storage)
{
    EXPECT_CALL(bondStorage, RemoveBondIf(testing::_))
        .WillOnce([this](const infra::Function<bool(const services::Bond&)>& onBond)
            {
                EXPECT_CALL(absoluteStorage, IsBondStored(gapAddress1)).WillOnce(testing::Return(false));
                EXPECT_THAT(onBond(bond1), testing::IsTrue());

                EXPECT_CALL(absoluteStorage, IsBondStored(gapAddress2)).WillOnce(testing::Return(true));
                EXPECT_THAT(onBond(bond2), testing::IsFalse());
            });

    EXPECT_CALL(absoluteStorage, IterateBondedDevices(testing::_));

    services::BondStorageSynchronizerImpl bondStorageSynchronizer(absoluteStorage, bondStorage);
}

TEST_F(BondStorageSynchronizerTest, construction_removes_bond_from_absolute_storage_when_not_in_bond_storage)
{
    EXPECT_CALL(bondStorage, RemoveBondIf(testing::_));

    EXPECT_CALL(absoluteStorage, IterateBondedDevices(testing::_))
        .WillOnce([this](const infra::Function<void(const services::GapAddress&)>& onAddress)
            {
                EXPECT_CALL(bondStorage, GetBond(services::Role::central, gapAddress1))
                    .WillOnce(testing::Return(std::optional<services::Bond>{}));
                EXPECT_CALL(bondStorage, GetBond(services::Role::peripheral, gapAddress1))
                    .WillOnce(testing::Return(std::optional<services::Bond>{}));
                EXPECT_CALL(absoluteStorage, RemoveBond(gapAddress1));
                onAddress(gapAddress1);
            });

    services::BondStorageSynchronizerImpl bondStorageSynchronizer(absoluteStorage, bondStorage);
}

TEST_F(BondStorageSynchronizerTest, construction_keeps_bond_in_absolute_storage_when_present_in_bond_storage)
{
    EXPECT_CALL(bondStorage, RemoveBondIf(testing::_));

    EXPECT_CALL(absoluteStorage, IterateBondedDevices(testing::_))
        .WillOnce([this](const infra::Function<void(const services::GapAddress&)>& onAddress)
            {
                EXPECT_CALL(bondStorage, GetBond(services::Role::central, gapAddress1))
                    .WillOnce(testing::Return(std::optional<services::Bond>{ bond1 }));
                onAddress(gapAddress1);
            });

    services::BondStorageSynchronizerImpl bondStorageSynchronizer(absoluteStorage, bondStorage);
}

class BondStorageSynchronizerTestWithConstruction
    : public BondStorageSynchronizerTest
{
public:
    void ExpectSyncBondStorages()
    {
        EXPECT_CALL(bondStorage, RemoveBondIf(testing::_));
        EXPECT_CALL(absoluteStorage, IterateBondedDevices(testing::_));
    }

    infra::Execute execute{ [this]()
        {
            ExpectSyncBondStorages();
        } };
    services::BondStorageSynchronizerImpl bondStorageSynchronizer{ absoluteStorage, bondStorage };
};

TEST_F(BondStorageSynchronizerTestWithConstruction, construction_notifies_bondstorages_of_creation_checks_max_number_of_bonds_and_synchronises_the_storages)
{
}

TEST_F(BondStorageSynchronizerTestWithConstruction, add_bond_is_forwarded_to_bond_storage)
{
    EXPECT_CALL(bondStorage, GetBond(services::Role::central, bond1.address)).WillOnce(testing::Return(std::optional<services::Bond>{}));
    EXPECT_CALL(bondStorage, GetBond(services::Role::peripheral, bond1.address)).WillOnce(testing::Return(std::optional<services::Bond>{}));
    EXPECT_CALL(bondStorage, AddBond(services::Role::peripheral, bond1));
    bondStorageSynchronizer.AddBond(services::Role::peripheral, bond1);
}

#ifndef EMIL_MUTATION_TESTING
TEST_F(BondStorageSynchronizerTestWithConstruction, add_bond_asserts_when_bond_already_exists_for_central_role)
{
    ON_CALL(bondStorage, GetBond(services::Role::central, bond1.address)).WillByDefault(testing::Return(std::optional<services::Bond>{ bond1 }));
    EXPECT_DEATH(bondStorageSynchronizer.AddBond(services::Role::peripheral, bond1), "");
}

TEST_F(BondStorageSynchronizerTestWithConstruction, add_bond_asserts_when_bond_already_exists_for_peripheral_role)
{
    ON_CALL(bondStorage, GetBond(services::Role::central, bond1.address)).WillByDefault(testing::Return(std::optional<services::Bond>{}));
    ON_CALL(bondStorage, GetBond(services::Role::peripheral, bond1.address)).WillByDefault(testing::Return(std::optional<services::Bond>{ bond1 }));
    EXPECT_DEATH(bondStorageSynchronizer.AddBond(services::Role::peripheral, bond1), "");
}
#endif

TEST_F(BondStorageSynchronizerTestWithConstruction, update_bond_name_is_forwarded_to_bond_storage)
{
    EXPECT_CALL(bondStorage, UpdateBondName(services::Role::peripheral, gapAddress1, infra::BoundedConstString("device1")));
    bondStorageSynchronizer.UpdateBondName(services::Role::peripheral, gapAddress1, "device1");
}

TEST_F(BondStorageSynchronizerTestWithConstruction, mark_as_recently_used_is_forwarded_to_bond_storage)
{
    EXPECT_CALL(bondStorage, MarkAsRecentlyUsed(services::Role::peripheral, gapAddress1));
    bondStorageSynchronizer.MarkAsRecentlyUsed(services::Role::peripheral, gapAddress1);
}

TEST_F(BondStorageSynchronizerTestWithConstruction, get_bond_is_forwarded_to_bond_storage)
{
    EXPECT_CALL(bondStorage, GetBond(services::Role::peripheral, gapAddress1)).WillOnce(testing::Return(std::optional<services::Bond>{ bond1 }));
    EXPECT_THAT(bondStorageSynchronizer.GetBond(services::Role::peripheral, gapAddress1), testing::Eq(std::optional<services::Bond>{ bond1 }));
}

TEST_F(BondStorageSynchronizerTestWithConstruction, remove_bond_is_forwarded_to_both_storages)
{
    EXPECT_CALL(absoluteStorage, RemoveBond(gapAddress1));
    EXPECT_CALL(bondStorage, RemoveBond(services::Role::peripheral, gapAddress1));
    bondStorageSynchronizer.RemoveBond(services::Role::peripheral, gapAddress1);
}

TEST_F(BondStorageSynchronizerTestWithConstruction, remove_all_bonds_for_role_removes_matching_bonds_from_absolute_storage)
{
    EXPECT_CALL(bondStorage, IterateBondedDevices(services::Role::peripheral, testing::_))
        .WillOnce([this](services::Role, const infra::Function<void(const services::Bond&)>& onBond)
            {
                EXPECT_CALL(absoluteStorage, RemoveBond(bond1.address));
                onBond(bond1);
            });
    EXPECT_CALL(bondStorage, RemoveAllBondsForRole(services::Role::peripheral));
    bondStorageSynchronizer.RemoveAllBondsForRole(services::Role::peripheral);
}

TEST_F(BondStorageSynchronizerTestWithConstruction, remove_all_bonds_is_forwarded_to_both_storages)
{
    EXPECT_CALL(absoluteStorage, RemoveAllBonds());
    EXPECT_CALL(bondStorage, RemoveAllBonds());
    bondStorageSynchronizer.RemoveAllBonds();
}

TEST_F(BondStorageSynchronizerTestWithConstruction, get_max_number_of_bonds_returns_minimum_of_both_storages)
{
    EXPECT_THAT(bondStorageSynchronizer.GetMaxNumberOfBonds(), testing::Eq(maxNumberOfBonds));
}

TEST_F(BondStorageSynchronizerTestWithConstruction, allocate_interactable_bond_storage_accepts_allocations_up_to_the_maximum)
{
    bondStorageSynchronizer.AllocateInteractableBondStorage(maxNumberOfBonds);
}

#ifndef EMIL_MUTATION_TESTING
TEST_F(BondStorageSynchronizerTestWithConstruction, allocate_interactable_bond_storage_asserts_when_exceeding_the_maximum)
{
    EXPECT_DEATH(bondStorageSynchronizer.AllocateInteractableBondStorage(maxNumberOfBonds + 1), "");
}
#endif

TEST_F(BondStorageSynchronizerTestWithConstruction, get_number_of_bonds_for_role_is_forwarded_to_bond_storage)
{
    EXPECT_CALL(bondStorage, GetNumberOfBondsForRole(services::Role::peripheral)).WillOnce(testing::Return(2));
    EXPECT_THAT(bondStorageSynchronizer.GetNumberOfBondsForRole(services::Role::peripheral), testing::Eq(2u));
}

TEST_F(BondStorageSynchronizerTestWithConstruction, iterate_bonded_devices_is_forwarded_to_bond_storage)
{
    EXPECT_CALL(bondStorage, IterateBondedDevices(services::Role::peripheral, testing::_));
    bondStorageSynchronizer.IterateBondedDevices(services::Role::peripheral, [](const services::Bond&) {});
}
