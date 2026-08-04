#include "services/ble/GapPeripheralStateGuardDecorator.hpp"
#include "infra/util/ReallyAssert.hpp"

namespace services
{
    GapPeripheralStateGuardDecorator::GapPeripheralStateGuardDecorator(GapPeripheral& peripheral, GapPairing& pairing, GapBonding& bonding)
        : GapPeripheralDecorator(peripheral)
        , GapPairingDecorator(pairing)
        , GapBondingDecorator(bonding)
    {}

    void GapPeripheralStateGuardDecorator::StateChanged(GapState newState)
    {
        state = newState;
        GapPeripheralDecorator::StateChanged(newState);
    }

    void GapPeripheralStateGuardDecorator::Advertise(GapAdvertisementType type, AdvertisementIntervalMultiplier multiplier)
    {
        really_assert(state == GapState::standby);
        GapPeripheralDecorator::Advertise(type, multiplier);
    }

    void GapPeripheralStateGuardDecorator::SetAdvertisementData(infra::ConstByteRange data)
    {
        really_assert(state == GapState::standby);
        GapPeripheralDecorator::SetAdvertisementData(data);
    }

    void GapPeripheralStateGuardDecorator::SetScanResponseData(infra::ConstByteRange data)
    {
        really_assert(state == GapState::standby);
        GapPeripheralDecorator::SetScanResponseData(data);
    }

    void GapPeripheralStateGuardDecorator::AllowPairing(bool allow)
    {
        really_assert(state == GapState::standby);
        GapPairingDecorator::AllowPairing(allow);
    }

    void GapPeripheralStateGuardDecorator::SetSecurityMode(SecurityMode mode, SecurityLevel level)
    {
        really_assert(state == GapState::standby);
        GapPairingDecorator::SetSecurityMode(mode, level);
    }

    void GapPeripheralStateGuardDecorator::SetIoCapabilities(IoCapabilities caps)
    {
        really_assert(state == GapState::standby);
        GapPairingDecorator::SetIoCapabilities(caps);
    }

    void GapPeripheralStateGuardDecorator::NumericComparisonConfirm(bool accept)
    {
        really_assert(state == GapState::standby || state == GapState::connected);
        GapPairingDecorator::NumericComparisonConfirm(accept);
    }

    void GapPeripheralStateGuardDecorator::RemoveAllBonds()
    {
        really_assert(state == GapState::standby);
        GapBondingDecorator::RemoveAllBonds();
    }

    void GapPeripheralStateGuardDecorator::RemoveOldestBond()
    {
        really_assert(state == GapState::standby);
        GapBondingDecorator::RemoveOldestBond();
    }
}
