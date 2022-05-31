#include "gmock/gmock.h"
#include "services/ble/Gatt.hpp"

TEST(UuidTest, should_support_uuid16)
{
    services::Gatt::Uuid uuid16{services::Gatt::Uuid16{0x180A}};

    EXPECT_EQ(0x180A, uuid16.Get<services::Gatt::Uuid16>());
    EXPECT_TRUE(uuid16.Is<services::Gatt::Uuid16>());
}

TEST(GattTest, characteristic_supports_different_uuid_lengths)
{
    services::GattService s{services::Gatt::Uuid16{}};
    services::GattCharacteristic a{s, services::Gatt::Uuid16{0x180A}};
    services::GattCharacteristic b{s, services::Gatt::Uuid32{}};
    services::GattCharacteristic c{s, services::Gatt::Uuid128{}};

    EXPECT_EQ(0x180A, a.Type().Get<services::Gatt::Uuid16>());
}

TEST(GattTest, should_add_characteristic_to_service)
{
    services::GattService s{services::Gatt::Uuid16{}};
    services::GattCharacteristic a{s, services::Gatt::Uuid16{0x180A}};
    services::GattCharacteristic b{s, services::Gatt::Uuid16{0x180B}};

    EXPECT_FALSE(s.Characteristics().empty());
    EXPECT_EQ(0x180B, s.Characteristics().front().Type().Get<services::Gatt::Uuid16>());
}
