#ifndef SERVICES_GAP_PERIPHERAL_INTERVAL_DECORATOR_HPP
#define SERVICES_GAP_PERIPHERAL_INTERVAL_DECORATOR_HPP

#include "services/ble/Gap.hpp"
#include <optional>

namespace services
{
    class GapPeripheralIntervalController
    {
    public:
        virtual void SwitchToLongInterval() = 0;
        virtual void SwitchToUserInterval() = 0;
    };

    class GapPeripheralIntervalDecorator
        : public services::GapPeripheralDecorator
        , public GapPeripheralIntervalController
    {
    public:
        explicit GapPeripheralIntervalDecorator(GapPeripheral& subject);

        // Implementation of GapPeripheralIntervalController
        void SwitchToLongInterval() override;
        void SwitchToUserInterval() override;

        // Implementation of GapPeripheralDecorator
        void Advertise(services::GapAdvertisementType type, AdvertisementIntervalMultiplier multiplier) override;
        void Standby() override;
        void StateChanged(services::GapState state) override;
        void SetConnectionParameters(const services::GapConnectionParameters& connParam) override;

    private:
        void SwitchInterval();
        void UpdateConnectionParameters();
        void HandlePendingAdvertise();

        static constexpr AdvertisementIntervalMultiplier longAdvMutiplier = 50;
        static const services::GapConnectionParameters defaultConnParam;
        static const services::GapConnectionParameters longConnParam;
        bool useLongInterval = false;
        services::GapState state = services::GapState::standby;
        std::optional<std::pair<services::GapAdvertisementType, AdvertisementIntervalMultiplier>> userAdvparam;
        bool pendingAdv = false;
        std::optional<services::GapConnectionParameters> userConnParam;
    };
}

#endif
