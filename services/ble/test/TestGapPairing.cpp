#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "services/ble/Gap.hpp"
#include "services/ble/test_doubles/GapPairingMock.hpp"
#include "services/ble/test_doubles/GapPairingObserverMock.hpp"
#include "gmock/gmock.h"

namespace services
{
    namespace
    {
        class GapPairingDecoratorTest
            : public testing::Test
        {
        public:
            GapPairingMock gapPairing;
            GapPairingDecorator decorator{ gapPairing };
            GapPairingObserverMock gapPairingObserver{ decorator };
        };
    }

    TEST_F(GapPairingDecoratorTest, forward_all_events_to_observers)
    {
        EXPECT_CALL(gapPairingObserver, DisplayPasskey(::testing::Eq(11111), ::testing::IsTrue()));
        EXPECT_CALL(gapPairingObserver, PairingSuccessfullyCompleted());
        EXPECT_CALL(gapPairingObserver, PairingFailed(::testing::TypedEq<services::GapPairingObserver::PairingErrorType>(services::GapPairingObserver::PairingErrorType::numericComparisonFailed)));

        gapPairing.NotifyObservers([](GapPairingObserver& obs)
            {
                obs.DisplayPasskey(11111, true);
                obs.PairingSuccessfullyCompleted();
                obs.PairingFailed(services::GapPairingObserver::PairingErrorType::numericComparisonFailed);
            });
    }

    TEST_F(GapPairingDecoratorTest, forward_all_calls_to_subject)
    {
        EXPECT_CALL(gapPairing, Pair());
        decorator.Pair();

        EXPECT_CALL(gapPairing, AllowPairing(::testing::IsTrue()));
        decorator.AllowPairing(true);

        EXPECT_CALL(gapPairing, SetSecurityMode(::testing::TypedEq<services::GapPairing::SecurityMode>(services::GapPairing::SecurityMode::mode1), ::testing::TypedEq<services::GapPairing::SecurityLevel>(services::GapPairing::SecurityLevel::level1)));
        decorator.SetSecurityMode(services::GapPairing::SecurityMode::mode1, services::GapPairing::SecurityLevel::level1);

        EXPECT_CALL(gapPairing, SetIoCapabilities(::testing::TypedEq<services::GapPairing::IoCapabilities>(services::GapPairing::IoCapabilities::none)));
        decorator.SetIoCapabilities(services::GapPairing::IoCapabilities::none);

        EXPECT_CALL(gapPairing, AuthenticateWithPasskey(::testing::Eq(11111)));
        decorator.AuthenticateWithPasskey(11111);

        EXPECT_CALL(gapPairing, NumericComparisonConfirm(::testing::IsFalse()));
        decorator.NumericComparisonConfirm(false);
    }
}
