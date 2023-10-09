#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/stream/StringOutputStream.hpp"
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

TEST(GattInsertionOperatorPropertyFlagsTest, property_flags_overload_operator)
{
    infra::StringOutputStream::WithStorage<128> stream;

    services::GattCharacteristic::PropertyFlags properties = services::GattCharacteristic::PropertyFlags::broadcast |
                                                             services::GattCharacteristic::PropertyFlags::read |
                                                             services::GattCharacteristic::PropertyFlags::writeWithoutResponse |
                                                             services::GattCharacteristic::PropertyFlags::write |
                                                             services::GattCharacteristic::PropertyFlags::notify |
                                                             services::GattCharacteristic::PropertyFlags::indicate |
                                                             services::GattCharacteristic::PropertyFlags::signedWrite |
                                                             services::GattCharacteristic::PropertyFlags::extended;

    stream << properties;

    EXPECT_EQ("[|broadcast||read||writeWithoutResponse||write||notify||indicate||signedWrite||extended|]", stream.Storage());
}

TEST(GattInsertionOperatorPropertyFlagsTest, property_flags_overload_operator_flag_none)
{
    infra::StringOutputStream::WithStorage<128> stream;

    services::GattCharacteristic::PropertyFlags properties = services::GattCharacteristic::PropertyFlags::none;

    stream << properties;

    EXPECT_EQ("[]", stream.Storage());
}

TEST(GattInsertionOperatorUuidTest, uuid_overload_operator)
{
    infra::StringOutputStream::WithStorage<128> stream;

    services::AttAttribute::Uuid128 uuid128{ { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 } };

    stream << "Uuid16: " << services::AttAttribute::Uuid(uuid16) << ", Uuid128: " << services::AttAttribute::Uuid(uuid128);

    EXPECT_EQ("Uuid16: [42], Uuid128: [100f0e0d0c0b0a090807060504030201]", stream.Storage());
}