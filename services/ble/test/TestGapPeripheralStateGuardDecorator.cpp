#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "services/ble/GapPeripheralStateGuardDecorator.hpp"
#include "services/ble/test_doubles/GapBondingMock.hpp"
#include "services/ble/test_doubles/GapBondingObserverMock.hpp"
#include "services/ble/test_doubles/GapPairingMock.hpp"
#include "services/ble/test_doubles/GapPairingObserverMock.hpp"
#include "services/ble/test_doubles/GapPeripheralMock.hpp"
#include "services/ble/test_doubles/GapPeripheralObserverMock.hpp"
#include "gmock/gmock.h"

class GapPeripheralStateGuardDecoratorTest
    : public testing::Test
{
public:
    void SetState(services::GapState newState)
    {
        EXPECT_CALL(peripheralObserver, StateChanged(newState));
        stateGuard.StateChanged(newState);
    }

    testing::StrictMock<services::GapPeripheralMock> peripheral;
    testing::StrictMock<services::GapPairingMock> pairing;
    testing::StrictMock<services::GapBondingMock> bonding;
    services::GapPeripheralStateGuardDecorator stateGuard{ peripheral, pairing, bonding };
    testing::StrictMock<services::GapPeripheralObserverMock> peripheralObserver{ stateGuard };
    testing::StrictMock<services::GapPairingObserverMock> pairingObserver{ stateGuard };
    testing::StrictMock<services::GapBondingObserverMock> bondingObserver{ stateGuard };

    const std::array<uint8_t, 3> data{ 1, 2, 3 };
};

TEST_F(GapPeripheralStateGuardDecoratorTest, advertise_forwards_in_standby)
{
    EXPECT_CALL(peripheral, Advertise(services::GapAdvertisementType::advInd, 100));

    stateGuard.Advertise(services::GapAdvertisementType::advInd, 100);
}

TEST_F(GapPeripheralStateGuardDecoratorTest, set_advertisement_data_forwards_in_standby)
{
    EXPECT_CALL(peripheral, SetAdvertisementData(infra::ContentsEqual(data)));

    stateGuard.SetAdvertisementData(data);
}

TEST_F(GapPeripheralStateGuardDecoratorTest, set_scan_response_data_forwards_in_standby)
{
    EXPECT_CALL(peripheral, SetScanResponseData(infra::ContentsEqual(data)));

    stateGuard.SetScanResponseData(data);
}

TEST_F(GapPeripheralStateGuardDecoratorTest, standby_forwards_in_all_states)
{
    EXPECT_CALL(peripheral, Standby()).Times(3);

    stateGuard.Standby();

    SetState(services::GapState::advertising);
    stateGuard.Standby();

    SetState(services::GapState::connected);
    stateGuard.Standby();
}

TEST_F(GapPeripheralStateGuardDecoratorTest, state_changed_is_forwarded_to_observer)
{
    EXPECT_CALL(peripheralObserver, StateChanged(services::GapState::advertising));

    stateGuard.StateChanged(services::GapState::advertising);
}

TEST_F(GapPeripheralStateGuardDecoratorTest, allow_pairing_forwards_in_standby)
{
    EXPECT_CALL(pairing, AllowPairing(true));

    stateGuard.AllowPairing(true);
}

TEST_F(GapPeripheralStateGuardDecoratorTest, set_security_mode_forwards_in_standby)
{
    EXPECT_CALL(pairing, SetSecurityMode(services::GapPairing::SecurityMode::mode1, services::GapPairing::SecurityLevel::level2));

    stateGuard.SetSecurityMode(services::GapPairing::SecurityMode::mode1, services::GapPairing::SecurityLevel::level2);
}

TEST_F(GapPeripheralStateGuardDecoratorTest, set_io_capabilities_forwards_in_standby)
{
    EXPECT_CALL(pairing, SetIoCapabilities(services::GapPairing::IoCapabilities::displayYesNo));

    stateGuard.SetIoCapabilities(services::GapPairing::IoCapabilities::displayYesNo);
}

TEST_F(GapPeripheralStateGuardDecoratorTest, numeric_comparison_confirm_forwards_in_standby)
{
    EXPECT_CALL(pairing, NumericComparisonConfirm(true));

    stateGuard.NumericComparisonConfirm(true);
}

TEST_F(GapPeripheralStateGuardDecoratorTest, numeric_comparison_confirm_forwards_in_connected)
{
    SetState(services::GapState::connected);

    EXPECT_CALL(pairing, NumericComparisonConfirm(true));

    stateGuard.NumericComparisonConfirm(true);
}

TEST_F(GapPeripheralStateGuardDecoratorTest, remove_all_bonds_forwards_in_standby)
{
    EXPECT_CALL(bonding, RemoveAllBonds());

    stateGuard.RemoveAllBonds();
}

TEST_F(GapPeripheralStateGuardDecoratorTest, remove_oldest_bond_forwards_in_standby)
{
    EXPECT_CALL(bonding, RemoveOldestBond());

    stateGuard.RemoveOldestBond();
}

TEST_F(GapPeripheralStateGuardDecoratorTest, advertise_asserts_when_advertising)
{
    SetState(services::GapState::advertising);

    EXPECT_DEATH(stateGuard.Advertise(services::GapAdvertisementType::advInd, 100), "");
}

TEST_F(GapPeripheralStateGuardDecoratorTest, advertise_asserts_when_connected)
{
    SetState(services::GapState::connected);

    EXPECT_DEATH(stateGuard.Advertise(services::GapAdvertisementType::advInd, 100), "");
}

TEST_F(GapPeripheralStateGuardDecoratorTest, set_advertisement_data_asserts_when_advertising)
{
    SetState(services::GapState::advertising);

    EXPECT_DEATH(stateGuard.SetAdvertisementData(data), "");
}

TEST_F(GapPeripheralStateGuardDecoratorTest, set_scan_response_data_asserts_when_connected)
{
    SetState(services::GapState::connected);

    EXPECT_DEATH(stateGuard.SetScanResponseData(data), "");
}

TEST_F(GapPeripheralStateGuardDecoratorTest, allow_pairing_asserts_when_advertising)
{
    SetState(services::GapState::advertising);

    EXPECT_DEATH(stateGuard.AllowPairing(true), "");
}

TEST_F(GapPeripheralStateGuardDecoratorTest, set_security_mode_asserts_when_connected)
{
    SetState(services::GapState::connected);

    EXPECT_DEATH(stateGuard.SetSecurityMode(services::GapPairing::SecurityMode::mode1, services::GapPairing::SecurityLevel::level2), "");
}

TEST_F(GapPeripheralStateGuardDecoratorTest, set_io_capabilities_asserts_when_advertising)
{
    SetState(services::GapState::advertising);

    EXPECT_DEATH(stateGuard.SetIoCapabilities(services::GapPairing::IoCapabilities::displayYesNo), "");
}

TEST_F(GapPeripheralStateGuardDecoratorTest, numeric_comparison_confirm_asserts_when_advertising)
{
    SetState(services::GapState::advertising);

    EXPECT_DEATH(stateGuard.NumericComparisonConfirm(true), "");
}

TEST_F(GapPeripheralStateGuardDecoratorTest, remove_all_bonds_asserts_when_connected)
{
    SetState(services::GapState::connected);

    EXPECT_DEATH(stateGuard.RemoveAllBonds(), "");
}

TEST_F(GapPeripheralStateGuardDecoratorTest, remove_oldest_bond_asserts_when_advertising)
{
    SetState(services::GapState::advertising);

    EXPECT_DEATH(stateGuard.RemoveOldestBond(), "");
}

TEST_F(GapPeripheralStateGuardDecoratorTest, methods_allowed_again_after_returning_to_standby)
{
    SetState(services::GapState::advertising);
    SetState(services::GapState::standby);

    EXPECT_CALL(peripheral, Advertise(services::GapAdvertisementType::advInd, 100));
    stateGuard.Advertise(services::GapAdvertisementType::advInd, 100);

    EXPECT_CALL(pairing, AllowPairing(true));
    stateGuard.AllowPairing(true);

    EXPECT_CALL(bonding, RemoveAllBonds());
    stateGuard.RemoveAllBonds();
}
