#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "services/ble/Gap.hpp"
#include "services/ble/test_doubles/GapCentralMock.hpp"
#include "services/ble/test_doubles/GapCentralObserverMock.hpp"
#include "services/ble/test_doubles/GapPeripheralMock.hpp"
#include "services/ble/test_doubles/GapPeripheralObserverMock.hpp"
#include "gmock/gmock.h"

namespace services
{
    namespace
    {
        class GapCentralDecoratorTest
            : public testing::Test
        {
        public:
            GapCentralMock gap;
            GapCentralDecorator decorator{ gap };
            GapCentralObserverMock gapObserver{ decorator };
        };
    }

    MATCHER_P(MacAddressContentsEqual, x, negation ? "Contents not equal" : "Contents are equal")
    {
        return infra::ContentsEqual(infra::MakeRange(x), infra::MakeRange(arg));
    }

    MATCHER_P(ObjectContentsEqual, x, negation ? "Contents not equal" : "Contents are equal")
    {
        return (x.eventType == arg.eventType) && (x.addressType == arg.addressType) && (x.address == arg.address) && (x.rssi == arg.rssi);
    }

    TEST_F(GapCentralDecoratorTest, forward_all_state_changed_events_to_observers)
    {
        EXPECT_CALL(gapObserver, StateChanged(GapState::connected));
        EXPECT_CALL(gapObserver, StateChanged(GapState::scanning));
        EXPECT_CALL(gapObserver, StateChanged(GapState::standby));

        gap.NotifyObservers([](GapCentralObserver& obs) {
            obs.StateChanged(GapState::connected);
            obs.StateChanged(GapState::scanning);
            obs.StateChanged(GapState::standby);
        });
    }

    TEST_F(GapCentralDecoratorTest, forward_device_discovered_event_to_observers)
    {
        GapAdvertisingReport deviceDiscovered{ GapAdvertisingEventType::advInd, GapAdvertisingEventAddressType::publicDeviceAddress, hal::MacAddress{ 0, 1, 2, 3, 4, 5 }, infra::ConstByteRange(), -75 };

        EXPECT_CALL(gapObserver, DeviceDiscovered(ObjectContentsEqual(deviceDiscovered)));

        gap.NotifyObservers([&deviceDiscovered](GapCentralObserver& obs) {
            obs.DeviceDiscovered(deviceDiscovered);
        });
    }

    TEST_F(GapCentralDecoratorTest, forward_all_authentication_events_to_observers)
    {
        EXPECT_CALL(gapObserver, AuthenticationSuccessfullyCompleted());
        EXPECT_CALL(gapObserver, AuthenticationFailed(GapAuthenticationErrorType::authenticationRequirementsNotMet));
        EXPECT_CALL(gapObserver, AuthenticationFailed(GapAuthenticationErrorType::unknown));

        gap.NotifyObservers([](GapCentralObserver& obs) {
            obs.AuthenticationSuccessfullyCompleted();
            obs.AuthenticationFailed(GapAuthenticationErrorType::authenticationRequirementsNotMet);
            obs.AuthenticationFailed(GapAuthenticationErrorType::unknown);
        });
    }

    TEST_F(GapCentralDecoratorTest, forward_all_calls_to_subject)
    {
        hal::MacAddress macAddress{ 0, 1, 2, 3, 4, 5 };

        EXPECT_CALL(gap, Connect(MacAddressContentsEqual(macAddress), services::GapDeviceAddressType::publicAddress));
        decorator.Connect(macAddress, services::GapDeviceAddressType::publicAddress);

        EXPECT_CALL(gap, Disconnect());
        decorator.Disconnect();

        EXPECT_CALL(gap, SetAddress(MacAddressContentsEqual(macAddress), GapDeviceAddressType::publicAddress));
        decorator.SetAddress(macAddress, GapDeviceAddressType::publicAddress);

        EXPECT_CALL(gap, StartDeviceDiscovery());
        decorator.StartDeviceDiscovery();

        EXPECT_CALL(gap, StopDeviceDiscovery());
        decorator.StopDeviceDiscovery();
    }
}
