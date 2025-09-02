#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/MemoryRange.hpp"
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
            hal::MacAddress macAddress = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };
            std::array<uint8_t, 16> temporary = { 0x7a, 0x1c, 0x3e, 0x5d, 0x92, 0x4b, 0x6f, 0x2a, 0x8e, 0x13, 0xbc, 0x77, 0x5f, 0x20, 0xd4, 0x61 };
            std::array<uint8_t, 16> random = { 0x2e, 0x48, 0x9d, 0x25, 0x47, 0x58, 0x53, 0x69, 0x84, 0x29, 0xc9, 0x8b, 0xd3, 0xa6, 0x41, 0xcd };
            std::array<uint8_t, 16> confirm = { 0xf4, 0x2c, 0xcc, 0x10, 0x69, 0x86, 0x95, 0x2e, 0x00, 0x89, 0x4d, 0x50, 0x3f, 0xab, 0x93, 0x39 };

            GapPairingMock gapPairing;
            GapPairingDecorator decorator{ gapPairing };
            GapPairingObserverMock gapPairingObserver{ decorator };
        };
    }

    MATCHER_P(OutOfBandDataContentsEqual, x, negation ? "Contents not equal" : "Contents are equal")
    {
        return x.macAddress == arg.macAddress && x.addressType == arg.addressType && infra::ContentsEqual(x.randomData, arg.randomData) && infra::ContentsEqual(x.confirmData, arg.confirmData);
    }

    TEST_F(GapPairingDecoratorTest, forward_all_events_to_observers)
    {
        EXPECT_CALL(gapPairingObserver, DisplayPasskey(::testing::Eq(11111), ::testing::IsTrue()));
        EXPECT_CALL(gapPairingObserver, PairingSuccessfullyCompleted());
        EXPECT_CALL(gapPairingObserver, PairingFailed(::testing::TypedEq<GapPairingObserver::PairingErrorType>(GapPairingObserver::PairingErrorType::numericComparisonFailed)));
        EXPECT_CALL(gapPairingObserver, OutOfBandDataGenerated(OutOfBandDataContentsEqual(services::GapOutOfBandData{ macAddress, GapDeviceAddressType::publicAddress, infra::MakeByteRange(random), infra::MakeByteRange(confirm) })));

        gapPairing.NotifyObservers([this](GapPairingObserver& obs)
            {
                obs.DisplayPasskey(11111, true);
                obs.PairingSuccessfullyCompleted();
                obs.PairingFailed(GapPairingObserver::PairingErrorType::numericComparisonFailed);
                obs.OutOfBandDataGenerated(GapOutOfBandData{ macAddress, GapDeviceAddressType::publicAddress, infra::MakeByteRange(random), infra::MakeByteRange(confirm) });
            });
    }

    TEST_F(GapPairingDecoratorTest, forward_all_calls_to_subject)
    {
        EXPECT_CALL(gapPairing, PairAndBond());
        decorator.PairAndBond();

        EXPECT_CALL(gapPairing, AllowPairing(::testing::IsTrue()));
        decorator.AllowPairing(true);

        EXPECT_CALL(gapPairing, SetSecurityMode(::testing::TypedEq<services::GapPairing::SecurityMode>(services::GapPairing::SecurityMode::mode1), ::testing::TypedEq<services::GapPairing::SecurityLevel>(services::GapPairing::SecurityLevel::level1)));
        decorator.SetSecurityMode(services::GapPairing::SecurityMode::mode1, services::GapPairing::SecurityLevel::level1);

        EXPECT_CALL(gapPairing, SetIoCapabilities(::testing::TypedEq<services::GapPairing::IoCapabilities>(services::GapPairing::IoCapabilities::none)));
        decorator.SetIoCapabilities(services::GapPairing::IoCapabilities::none);

        EXPECT_CALL(gapPairing, GenerateOutOfBandData());
        decorator.GenerateOutOfBandData();

        EXPECT_CALL(gapPairing, SetOutOfBandData(OutOfBandDataContentsEqual(services::GapOutOfBandData{ hal::MacAddress(), GapDeviceAddressType::publicAddress, infra::MakeByteRange(random), infra::MakeByteRange(confirm) })));
        decorator.SetOutOfBandData({ hal::MacAddress(), GapDeviceAddressType::publicAddress, infra::MakeByteRange(random), infra::MakeByteRange(confirm) });

        EXPECT_CALL(gapPairing, AuthenticateWithPasskey(::testing::Eq(11111)));
        decorator.AuthenticateWithPasskey(11111);

        EXPECT_CALL(gapPairing, NumericComparisonConfirm(::testing::IsFalse()));
        decorator.NumericComparisonConfirm(false);
    }
}
