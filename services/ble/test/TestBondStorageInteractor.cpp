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

TEST_F(BondStorageInteractorTest, add_bond_is_forwarded_with_role)
{
    EXPECT_CALL(bondStorageSynchroniser, GetBond(role, bond1.address)).WillOnce(testing::Return(std::optional<services::Bond>{}));
    EXPECT_CALL(bondStorageSynchroniser, GetNumberOfBondsForRole(role)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(bondStorageSynchroniser, AddBond(role, bond1));
    interactor.AddBond(bond1);
}

TEST_F(BondStorageInteractorTest, add_bond_is_ignored_when_bond_already_exists)
{
    EXPECT_CALL(bondStorageSynchroniser, GetBond(role, bond1.address)).WillOnce(testing::Return(std::optional<services::Bond>{ bond1 }));
    interactor.AddBond(bond1);
}

TEST_F(BondStorageInteractorTest, add_bond_removes_least_recently_used_bond_when_storage_is_full)
{
    EXPECT_CALL(bondStorageSynchroniser, GetBond(role, bond2.address)).WillOnce(testing::Return(std::optional<services::Bond>{}));
    EXPECT_CALL(bondStorageSynchroniser, GetNumberOfBondsForRole(role))
        .WillOnce(testing::Return(maxNumberOfBonds))
        .WillOnce(testing::Return(maxNumberOfBonds - 1));
    EXPECT_CALL(bondStorageSynchroniser, IterateBondedDevices(role, testing::_))
        .WillOnce([this](services::Role, const infra::Function<void(const services::Bond&)>& onBond)
            {
                onBond(bond1);
                onBond(bond2);
            });
    EXPECT_CALL(bondStorageSynchroniser, RemoveBond(role, bond1.address));
    EXPECT_CALL(bondStorageSynchroniser, AddBond(role, bond2));
    interactor.AddBond(bond2);
}

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
