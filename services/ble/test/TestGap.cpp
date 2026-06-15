#include "services/ble/Gap.hpp"
#include "gmock/gmock.h"

namespace services
{
    TEST(GapDeviceAddressTypeFromAddressTest, public_address_returns_public_address_type)
    {
        EXPECT_THAT(GapDeviceAddressTypeFromAddress({ 0x01, 0x02, 0x03, 0x04, 0x05, 0x00 }), testing::Eq(GapDeviceAddressType::publicAddress));
    }

    TEST(GapDeviceAddressTypeFromAddressTest, static_random_address_returns_random_address_type)
    {
        EXPECT_THAT(GapDeviceAddressTypeFromAddress({ 0x01, 0x02, 0x03, 0x04, 0x05, 0xC0 }), testing::Eq(GapDeviceAddressType::randomAddress));
    }
}
