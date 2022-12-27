#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "services/ble/Gap.hpp"
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
        EXPECT_CALL(gapObserver, StateUpdated(GapPeripheralState::Connected));
        EXPECT_CALL(gapObserver, StateUpdated(GapPeripheralState::Advertising));

        gap.NotifyObservers([](GapPeripheralObserver& obs) {
            obs.StateUpdated(GapPeripheralState::Connected);
            obs.StateUpdated(GapPeripheralState::Advertising);
        });
    }

    TEST_F(GapPeripheralDecoratorTest, forward_all_calls_to_subject)
    {
        EXPECT_CALL(gap, GetPublicAddress()).WillOnce(testing::Return(hal::MacAddress({ 0, 1, 2, 3, 4, 5 })));
        EXPECT_THAT(decorator.GetPublicAddress(), testing::Eq(hal::MacAddress({ 0, 1, 2, 3, 4, 5 })));

        std::array<uint8_t, 6> data{ 0, 1, 2, 3, 4, 5 };
        EXPECT_CALL(gap, SetAdvertisementData(infra::ContentsEqual(data)));
        decorator.SetAdvertisementData(data);

        EXPECT_CALL(gap, SetScanResponseData(infra::ContentsEqual(data)));
        decorator.SetScanResponseData(data);

        EXPECT_CALL(gap, Advertise(services::GapPeripheral::AdvertisementType::advNonconnInd, 32));
        decorator.Advertise(services::GapPeripheral::AdvertisementType::advNonconnInd, 32);

        EXPECT_CALL(gap, Standby());
        decorator.Standby();
    }
}