#include "gmock/gmock.h"
#include "services/ble/GattCharacteristicImpl.hpp"

class GattCharacteristicClientOperationsMock
    : public services::GattCharacteristicClientOperations
{
public:
    MOCK_CONST_METHOD2(Update, bool(const services::GattCharacteristicClientOperationsObserver& characteristic, infra::ConstByteRange data));
};

namespace
{
    static constexpr services::GattAttribute::Uuid16 uuid16{0x42};
    static constexpr services::GattAttribute::Uuid128 uuid128{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                                              0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    static constexpr uint16_t valueSize = 16;
}

TEST(GattTest, characteristic_supports_different_uuid_lengths)
{
    services::GattService s{uuid16};
    services::GattCharacteristicImpl a{s, uuid16, valueSize};
    services::GattCharacteristicImpl b{s, uuid128, valueSize};

    //EXPECT_EQ(0x42, a.Type().Get<services::GattAttribute::Uuid16>());
    EXPECT_EQ((std::array<uint8_t, 16>{
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
    }), b.Type().Get<services::GattAttribute::Uuid128>());
}

TEST(GattTest, should_add_characteristic_to_service)
{
    services::GattService s{uuid16};
    services::GattCharacteristicImpl a{s, uuid16, valueSize};
    services::GattCharacteristicImpl b{s, uuid16, valueSize};

    EXPECT_FALSE(s.Characteristics().empty());
    EXPECT_EQ(0x42, s.Characteristics().front().Type().Get<services::GattAttribute::Uuid16>());
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
    services::GattService service{uuid16};
    services::GattCharacteristicImpl characteristic{service, uuid16, valueSize};
};

MATCHER_P(ContentsEqual, x, negation ? "Contents not equal" : "Contents are equal") { return infra::ContentsEqual(infra::MakeStringByteRange(x), arg); }

TEST_F(GattCharacteristicTest, should_update_characteristic)
{
    EXPECT_CALL(operations, Update(testing::Ref(characteristic), ContentsEqual("string")));
    characteristic.Update(infra::MakeStringByteRange("string"), infra::emptyFunction);
}
