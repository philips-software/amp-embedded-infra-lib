#include "gmock/gmock.h"
#include "services/ble/Gatt.hpp"

TEST(GattTest, characteristic_supports_different_uuid_lengths)
{
    services::GattCharacteristic a{services::Gatt::Uuid16{std::numeric_limits<services::Gatt::Uuid16>::max()}};
    services::GattCharacteristic b{services::Gatt::Uuid32{std::numeric_limits<services::Gatt::Uuid32>::max()}};
    services::GattCharacteristic c{services::Gatt::Uuid128{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
                                                           0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80}};
}

TEST(GattTest, should_add_characteristic_to_service)
{
    services::GattCharacteristic a{services::Gatt::Uuid16{std::numeric_limits<services::Gatt::Uuid16>::max()}};
    services::GattCharacteristic::WithStorage<5> b{services::Gatt::Uuid16{0x10}};

    services::GattService s{services::Gatt::Uuid16{1}, a, b};

    s.AddCharacteristic(services::GattCharacteristic{services::Gatt::Uuid128{0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
                                                                             0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80}});
}
