#include "infra/util/Function.hpp"
#include "services/ble/BondStorageInteractor.hpp"
#include "services/ble/test_doubles/BondStorageSynchronizerMock.hpp"
#include "gmock/gmock.h"

class BondStorageInteractorTest
    : public testing::Test
{
public:
    static services::GapAddress MakeGapAddress(hal::MacAddress address)
    {
        return services::GapAddress{ address, services::GapDeviceAddressType::publicAddress };
    }

    static services::Bond MakeBond(const services::GapAddress& address, infra::BoundedConstString deviceName)
    {
        return services::Bond{ address, deviceName };
    }

    hal::MacAddress address1{ { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 } };
    hal::MacAddress address2{ { 0x06, 0x07, 0x08, 0x09, 0x10, 0x11 } };

    services::GapAddress gapAddress1{ MakeGapAddress(address1) };
    services::GapAddress gapAddress2{ MakeGapAddress(address2) };
    services::Bond bond1{ MakeBond(gapAddress1, "device1") };
    services::Bond bond2{ MakeBond(gapAddress2, "device2") };

    services::Role role = services::Role::peripheral;
    uint32_t maxNumberOfBonds = 3;

    testing::StrictMock<services::BondStorageSynchronizerMock> bondStorageSynchroniser;

    infra::Execute execute{ [this]()
        {
            EXPECT_CALL(bondStorageSynchroniser, AllocateInteractableBondStorage(maxNumberOfBonds));
        } };
    services::BondStorageInteractor interactor{ role, bondStorageSynchroniser, maxNumberOfBonds };
};

TEST_F(BondStorageInteractorTest, construction_allocates_interactable_bond_storage)
{
}

TEST_F(BondStorageInteractorTest, construction_without_maximum_allocates_all_bonds_of_the_synchroniser)
{
    EXPECT_CALL(bondStorageSynchroniser, GetMaxNumberOfBonds()).WillOnce(testing::Return(5));
    EXPECT_CALL(bondStorageSynchroniser, AllocateInteractableBondStorage(5));
    services::BondStorageInteractor interactorWithoutMaximum{ services::Role::central, bondStorageSynchroniser };

    EXPECT_THAT(interactorWithoutMaximum.GetMaxNumberOfBonds(), testing::Eq(5u));
}

TEST_F(BondStorageInteractorTest, add_bond_is_forwarded_with_role)
{
    EXPECT_CALL(bondStorageSynchroniser, GetNumberOfBondsForRole(role)).WillOnce(testing::Return(1));
    EXPECT_CALL(bondStorageSynchroniser, GetBond(role, bond1.address)).WillOnce(testing::Return(std::optional<services::Bond>{}));
    EXPECT_CALL(bondStorageSynchroniser, AddBond(role, bond1));
    interactor.AddBond(bond1);
}

#ifndef EMIL_MUTATION_TESTING
TEST_F(BondStorageInteractorTest, add_bond_asserts_when_bond_already_exists)
{
    ON_CALL(bondStorageSynchroniser, GetNumberOfBondsForRole(role)).WillByDefault(testing::Return(1));
    ON_CALL(bondStorageSynchroniser, GetBond(role, bond1.address)).WillByDefault(testing::Return(std::optional<services::Bond>{ bond1 }));
    EXPECT_DEATH(interactor.AddBond(bond1), "");
}

TEST_F(BondStorageInteractorTest, add_bond_asserts_when_storage_is_full)
{
    ON_CALL(bondStorageSynchroniser, GetNumberOfBondsForRole(role)).WillByDefault(testing::Return(maxNumberOfBonds));
    EXPECT_DEATH(interactor.AddBond(bond1), "");
}
#endif

TEST_F(BondStorageInteractorTest, update_bond_name_is_forwarded_with_role)
{
    EXPECT_CALL(bondStorageSynchroniser, UpdateBondName(role, gapAddress1, infra::BoundedConstString("device1")));
    interactor.UpdateBondName(gapAddress1, "device1");
}

TEST_F(BondStorageInteractorTest, mark_as_recently_used_is_forwarded_with_role)
{
    EXPECT_CALL(bondStorageSynchroniser, MarkAsRecentlyUsed(role, gapAddress1));
    interactor.MarkAsRecentlyUsed(gapAddress1);
}

TEST_F(BondStorageInteractorTest, get_bond_is_forwarded_with_role)
{
    EXPECT_CALL(bondStorageSynchroniser, GetBond(role, gapAddress1)).WillOnce(testing::Return(std::optional<services::Bond>{ bond1 }));
    EXPECT_THAT(interactor.GetBond(gapAddress1), testing::Eq(std::optional<services::Bond>{ bond1 }));
}

TEST_F(BondStorageInteractorTest, remove_bond_is_forwarded_with_role)
{
    EXPECT_CALL(bondStorageSynchroniser, RemoveBond(role, gapAddress1));
    interactor.RemoveBond(gapAddress1);
}

TEST_F(BondStorageInteractorTest, remove_all_bonds_removes_all_bonds_for_role)
{
    EXPECT_CALL(bondStorageSynchroniser, RemoveAllBondsForRole(role));
    interactor.RemoveAllBonds();
}

TEST_F(BondStorageInteractorTest, iterate_bonded_devices_is_forwarded_with_role)
{
    EXPECT_CALL(bondStorageSynchroniser, IterateBondedDevices(role, testing::_));
    interactor.IterateBondedDevices([](const services::Bond&) {});
}

TEST_F(BondStorageInteractorTest, get_number_of_bonds_is_forwarded_with_role)
{
    EXPECT_CALL(bondStorageSynchroniser, GetNumberOfBondsForRole(role)).WillOnce(testing::Return(2));
    EXPECT_THAT(interactor.GetNumberOfBonds(), testing::Eq(2u));
}

TEST_F(BondStorageInteractorTest, get_max_number_of_bonds_returns_configured_maximum)
{
    EXPECT_THAT(interactor.GetMaxNumberOfBonds(), testing::Eq(maxNumberOfBonds));
}

TEST_F(BondStorageInteractorTest, full_returns_false_when_below_maximum)
{
    EXPECT_CALL(bondStorageSynchroniser, GetNumberOfBondsForRole(role)).WillOnce(testing::Return(maxNumberOfBonds - 1));
    EXPECT_THAT(interactor.Full(), testing::IsFalse());
}

TEST_F(BondStorageInteractorTest, full_returns_true_when_at_maximum)
{
    EXPECT_CALL(bondStorageSynchroniser, GetNumberOfBondsForRole(role)).WillOnce(testing::Return(maxNumberOfBonds));
    EXPECT_THAT(interactor.Full(), testing::IsTrue());
}

TEST_F(BondStorageInteractorTest, assert_bond_storages_are_in_sync_is_forwarded_with_role)
{
    EXPECT_CALL(bondStorageSynchroniser, AssertBondStoragesAreInSyncForRole(role));
    interactor.AssertBondStoragesAreInSync();
}

TEST_F(BondStorageInteractorTest, remove_least_recently_used_bond_removes_nothing_when_no_bonds_are_stored)
{
    EXPECT_CALL(bondStorageSynchroniser, IterateBondedDevices(role, testing::_));
    interactor.RemoveLeastRecentlyUsedBond();
}

TEST_F(BondStorageInteractorTest, remove_least_recently_used_bond_removes_first_bond_of_iteration)
{
    EXPECT_CALL(bondStorageSynchroniser, IterateBondedDevices(role, testing::_))
        .WillOnce([this](services::Role, const infra::Function<void(const services::Bond&)>& onBond)
            {
                onBond(bond1);
                onBond(bond2);
            });
    EXPECT_CALL(bondStorageSynchroniser, RemoveBond(role, gapAddress1));
    interactor.RemoveLeastRecentlyUsedBond();
}
