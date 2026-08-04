#ifndef SERVICES_GAP_PERIPHERAL_STATE_GUARD_DECORATOR_HPP
#define SERVICES_GAP_PERIPHERAL_STATE_GUARD_DECORATOR_HPP

#include "services/ble/Gap.hpp"

namespace services
{
    class GapPeripheralStateGuardDecorator
        : public GapPeripheralDecorator
        , public GapPairingDecorator
        , public GapBondingDecorator
    {
    public:
        GapPeripheralStateGuardDecorator(GapPeripheral& peripheral, GapPairing& pairing, GapBonding& bonding);

        // GapPeripheralDecorator overrides
        void StateChanged(GapState newState) override;
        void Advertise(GapAdvertisementType type, AdvertisementIntervalMultiplier multiplier) override;
        void SetAdvertisementData(infra::ConstByteRange data) override;
        void SetScanResponseData(infra::ConstByteRange data) override;

        // GapPairingDecorator overrides
        void AllowPairing(bool allow) override;
        void SetSecurityMode(SecurityMode mode, SecurityLevel level) override;
        void SetIoCapabilities(IoCapabilities caps) override;
        void NumericComparisonConfirm(bool accept) override;

        // GapBondingDecorator overrides
        void RemoveAllBonds() override;
        void RemoveOldestBond() override;

    private:
        GapState state = GapState::standby;
    };
}

#endif
