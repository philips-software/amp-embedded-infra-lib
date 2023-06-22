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
        class GapPeripheralDecoratorTest
            : public testing::Test
        {
        public:
            GapPeripheralMock gap;
            GapPeripheralDecorator decorator{ gap };
            GapPeripheralObserverMock gapObserver{ decorator };
        };
    }

    TEST_F(GapPeripheralDecoratorTest, forward_all_events_to_observers)
    {
        EXPECT_CALL(gapObserver, StateChanged(GapState::connected));
        EXPECT_CALL(gapObserver, StateChanged(GapState::advertising));

        gap.NotifyObservers([](GapPeripheralObserver& obs)
            {
                obs.StateChanged(GapState::connected);
                obs.StateChanged(GapState::advertising);
            });
    }

    TEST_F(GapPeripheralDecoratorTest, forward_all_calls_to_subject)
    {
        services::GapAddress address = { hal::MacAddress({ 5, 4, 3, 2, 1, 0 }), services::GapDeviceAddressType::publicAddress };
        EXPECT_CALL(gap, GetAddress()).WillOnce(testing::Return(address));
        EXPECT_THAT(decorator.GetAddress(), testing::Eq(address));

        services::GapAddress identityAddress = { hal::MacAddress({ 0, 1, 2, 3, 4, 5 }), services::GapDeviceAddressType::publicAddress };
        EXPECT_CALL(gap, GetIdentityAddress()).WillOnce(testing::Return(identityAddress));
        EXPECT_THAT(decorator.GetIdentityAddress(), testing::Eq(identityAddress));

        std::array<uint8_t, 6> data{ 0, 1, 2, 3, 4, 5 };
        EXPECT_CALL(gap, SetAdvertisementData(infra::ContentsEqual(data)));
        decorator.SetAdvertisementData(data);

        EXPECT_CALL(gap, SetScanResponseData(infra::ContentsEqual(data)));
        decorator.SetScanResponseData(data);

        EXPECT_CALL(gap, Advertise(services::GapAdvertisementType::advNonconnInd, 32));
        decorator.Advertise(services::GapAdvertisementType::advNonconnInd, 32);

        EXPECT_CALL(gap, Standby());
        decorator.Standby();
    }
}
