#include "gmock/gmock.h"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "services/ble/Gatt.hpp"

namespace
{
    services::AttAttribute::Uuid16 uuid16{0x42};
}

TEST(GattTest, service_has_handle_and_type)
{
    services::GattService s{uuid16};

    EXPECT_EQ(0x42, s.Type().Get<services::AttAttribute::Uuid16>());
    EXPECT_EQ(0, s.Handle());
}

TEST(GattTest, service_handle_is_updated)
{
    services::GattService s{uuid16};
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

TEST(GattTest, descriptor_handles_are_accesible)
{
    services::GattDescriptor d(uuid16, 0x56);

    EXPECT_EQ(0x56, d.Handle());
    EXPECT_EQ(0x42, d.Type().Get<services::AttAttribute::Uuid16>());
}
