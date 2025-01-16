#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "services/ble/Gap.hpp"
#include "services/ble/test_doubles/GapBondingMock.hpp"
#include "services/ble/test_doubles/GapBondingObserverMock.hpp"
#include "gmock/gmock.h"

namespace services
{
    namespace
    {
        class GapBondingDecoratorTest
            : public testing::Test
        {
        public:
            GapBondingMock gapBonding;
            GapBondingDecorator decorator{ gapBonding };
            GapBondingObserverMock gapBondingObserver{ decorator };
        };
    }

    TEST_F(GapBondingDecoratorTest, forward_all_events_to_observers)
    {
        EXPECT_CALL(gapBondingObserver, NumberOfBondsChanged(::testing::Eq(10)));

        gapBonding.NotifyObservers([](GapBondingObserver& obs)
            {
                obs.NumberOfBondsChanged(10);
            });
    }

    TEST_F(GapBondingDecoratorTest, forward_all_calls_to_subject)
    {
        EXPECT_CALL(gapBonding, RemoveAllBonds());
        decorator.RemoveAllBonds();

        EXPECT_CALL(gapBonding, RemoveOldestBond());
        decorator.RemoveOldestBond();

        EXPECT_CALL(gapBonding, GetMaxNumberOfBonds()).WillOnce(testing::Return(5));
        EXPECT_EQ(decorator.GetMaxNumberOfBonds(), 5);

        EXPECT_CALL(gapBonding, GetNumberOfBonds()).WillOnce(testing::Return(5));
        EXPECT_EQ(decorator.GetNumberOfBonds(), 5);

        hal::MacAddress mac = { 0x00, 0x1A, 0x7D, 0xDA, 0x71, 0x13 };
        services::GapDeviceAddressType addressType = services::GapDeviceAddressType::randomAddress;

        EXPECT_CALL(gapBonding, IsDeviceBonded(mac, addressType)).WillOnce(testing::Return(true));
        EXPECT_THAT(decorator.IsDeviceBonded(mac, addressType), testing::IsTrue());

        EXPECT_CALL(gapBonding, IsDeviceBonded(mac, addressType)).WillOnce(testing::Return(false));
        EXPECT_THAT(decorator.IsDeviceBonded(mac, addressType), testing::IsFalse());
    }
}
