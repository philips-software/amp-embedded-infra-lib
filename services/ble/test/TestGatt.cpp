#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "services/ble/Gatt.hpp"
#include "gmock/gmock.h"

namespace
{
    services::AttAttribute::Uuid16 uuid16{ 0x42 };
}

TEST(GattTest, service_has_handle_and_type)
{
    services::GattService s{ uuid16 };

    EXPECT_EQ(0x42, s.Type().Get<services::AttAttribute::Uuid16>());
    EXPECT_EQ(0, s.Handle());
    EXPECT_EQ(0, s.EndHandle());
    EXPECT_EQ(0, s.GetAttributeCount());
}

TEST(GattTest, service_handle_is_updated)
{
    services::GattService s{ uuid16 };
    s.Handle() = 0xAB;

    EXPECT_EQ(0xAB, s.Handle());
    EXPECT_EQ(0xAB, std::as_const(s).Handle());
}

TEST(GattTest, characteristic_handles_are_accesible)
{
    services::GattCharacteristic c;

    c.Handle() = 0xCD;
    c.ValueHandle() = 0xFE;

    EXPECT_EQ(0xCD, c.Handle());
    EXPECT_EQ(0xFE, c.ValueHandle());
}

TEST(GattTest, const_characteristic)
{
    const services::GattCharacteristic c{ uuid16, 0xCD, 0xFE, services::GattCharacteristic::PropertyFlags::none };

    EXPECT_EQ(0xCD, c.Handle());
    EXPECT_EQ(0xFE, c.ValueHandle());
    EXPECT_EQ(services::GattCharacteristic::PropertyFlags::none, c.Properties());
}