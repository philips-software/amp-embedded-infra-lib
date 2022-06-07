#include "gmock/gmock.h"
#include "services/ble/GattCharacteristicImpl.hpp"

class GattCharacteristicClientOperationsMock
    : public services::GattCharacteristicClientOperations
{
public:
    MOCK_CONST_METHOD2(Update, bool(const services::GattCharacteristicClientOperationsObserver& characteristic, infra::ConstByteRange data));
};

TEST(GattTest, characteristic_supports_different_uuid_lengths)
{
    services::GattService s{services::GattAttribute::Uuid16{}};
    services::GattCharacteristicImpl a{s, services::GattAttribute::Uuid16{0x180A}, 10};
    services::GattCharacteristicImpl b{s, services::GattAttribute::Uuid128{}, 10};

    EXPECT_EQ(0x180A, a.Type().Get<services::GattAttribute::Uuid16>());
}

TEST(GattTest, should_add_characteristic_to_service)
{
    services::GattService s{services::GattAttribute::Uuid16{}};
    services::GattCharacteristicImpl a{s, services::GattAttribute::Uuid16{0x180A}, 10};
    services::GattCharacteristicImpl b{s, services::GattAttribute::Uuid16{0x180B}, 10};

    EXPECT_FALSE(s.Characteristics().empty());
    EXPECT_EQ(0x180B, s.Characteristics().front().Type().Get<services::GattAttribute::Uuid16>());
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
    services::GattService service{services::GattAttribute::Uuid16{}};
    services::GattCharacteristicImpl characteristic{service, services::GattAttribute::Uuid16{}, 8};
};

MATCHER_P(ContentsEqual, x, negation ? "Contents not equal" : "Contents are equal") { return infra::ContentsEqual(infra::MakeStringByteRange(x), arg); }

TEST_F(GattCharacteristicTest, should_update_characteristic)
{
    EXPECT_CALL(operations, Update(testing::Ref(characteristic), ContentsEqual("string")));
    characteristic.Update(infra::MakeStringByteRange("string"), infra::emptyFunction);
}
