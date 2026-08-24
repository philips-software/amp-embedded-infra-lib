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

    services::GapAddress gapAddress1{ MakeGapAddress(address1) };
    services::Bond bond1{ MakeBond(gapAddress1, "device1") };

    services::Role role = services::Role::peripheral;
    uint32_t maxNumberOfBonds = 3;

    testing::StrictMock<services::BondStorageSynchronizerMock> bondStorageSynchroniser;
    services::BondStorageInteractor interactor{ role, bondStorageSynchroniser };
};

TEST_F(BondStorageInteractorTest, update_bond_is_forwarded_with_role)
{
    EXPECT_CALL(bondStorageSynchroniser, UpdateBond(role, bond1));
    interactor.UpdateBond(bond1);
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

TEST_F(BondStorageInteractorTest, get_max_number_of_bonds_is_forwarded)
{
    EXPECT_CALL(bondStorageSynchroniser, GetMaxNumberOfBonds()).WillOnce(testing::Return(maxNumberOfBonds));
    EXPECT_THAT(interactor.GetMaxNumberOfBonds(), testing::Eq(maxNumberOfBonds));
}
