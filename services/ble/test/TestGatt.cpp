#include "gmock/gmock.h"
#include "services/ble/Gatt.hpp"

class GattCharacteristicClientOperationsMock
    : public services::GattCharacteristicClientOperations
{
public:
    MOCK_CONST_METHOD2(Update, bool(const services::GattCharacteristicClientOperationsObserver& characteristic, infra::ConstByteRange data));
};

TEST(UuidTest, should_support_uuid16)
{
    services::Gatt::Uuid uuid16{services::Gatt::Uuid16{0x180A}};

    EXPECT_EQ(0x180A, uuid16.Get<services::Gatt::Uuid16>());
    EXPECT_TRUE(uuid16.Is<services::Gatt::Uuid16>());
}

TEST(GattTest, characteristic_supports_different_uuid_lengths)
{
    services::GattService s{services::Gatt::Uuid16{}};
    services::GattCharacteristic a{s, services::Gatt::Uuid16{0x180A}, 10};
    services::GattCharacteristic b{s, services::Gatt::Uuid32{}, 10};
    services::GattCharacteristic c{s, services::Gatt::Uuid128{}, 10};

    EXPECT_EQ(0x180A, a.Type().Get<services::Gatt::Uuid16>());
}

TEST(GattTest, should_add_characteristic_to_service)
{
    services::GattService s{services::Gatt::Uuid16{}};
    services::GattCharacteristic a{s, services::Gatt::Uuid16{0x180A}, 10};
    services::GattCharacteristic b{s, services::Gatt::Uuid16{0x180B}, 10};

    EXPECT_FALSE(s.Characteristics().empty());
    EXPECT_EQ(0x180B, s.Characteristics().front().Type().Get<services::Gatt::Uuid16>());
}

class GattCharacteristicTest
    : public testing::Test
{
public:
    GattCharacteristicTest()
    {
        characteristic.Attach(operations);
    }

    GattCharacteristicClientOperationsMock operations;
    services::GattService service{services::Gatt::Uuid16{}};
    services::GattCharacteristic characteristic{service, services::Gatt::Uuid16{}, 8};
};

MATCHER_P(ContentsEqual, x, negation ? "Contents not equal" : "Contents are equal") { return infra::ContentsEqual(infra::MakeStringByteRange(x), arg); }

TEST_F(GattCharacteristicTest, should_update_characteristic)
{
    EXPECT_CALL(operations, Update(testing::Ref(characteristic), ContentsEqual("string")));
    characteristic.Update(infra::MakeStringByteRange("string"), infra::emptyFunction);
}
