#include "gmock/gmock.h"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/ble/GattCharacteristicImpl.hpp"

class GattCharacteristicClientOperationsMock
    : public services::GattCharacteristicClientOperations
{
public:
    MOCK_CONST_METHOD2(Update, UpdateStatus(const services::GattCharacteristicClientOperationsObserver& characteristic, infra::ConstByteRange data));
};

namespace
{
    services::GattAttribute::Uuid16 uuid16{ 0x42 };
    services::GattAttribute::Uuid128 uuid128{ { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 } };
    static constexpr uint16_t valueSize = 16;

    using GattPropertyFlags = services::GattCharacteristic::PropertyFlags;
    using GattPermissionFlags = services::GattCharacteristic::PermissionFlags;
}

TEST(GattTest, service_has_handle_and_type)
{
    services::GattService s{ uuid16 };

    EXPECT_EQ(0x42, s.Type().Get<services::GattAttribute::Uuid16>());
    EXPECT_EQ(0, s.Handle());
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
    services::GattService s{ uuid16 };
    s.Handle() = 0xAB;
    services::GattCharacteristicImpl c{ s, uuid16, valueSize };
    c.Handle() = 0xCD;

    EXPECT_EQ(0xAB, c.ServiceHandle());
    EXPECT_EQ(0xCD, c.CharacteristicHandle());
}

TEST(GattTest, characteristic_supports_different_uuid_lengths)
{
    services::GattService s{ uuid16 };
    services::GattCharacteristicImpl a{ s, uuid16, valueSize };
    services::GattCharacteristicImpl b{ s, uuid128, valueSize };

    EXPECT_EQ(0x42, a.Type().Get<services::GattAttribute::Uuid16>());
    EXPECT_EQ((infra::BigEndian<std::array<uint8_t, 16>>{ { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                  0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 } }),
        b.Type().Get<services::GattAttribute::Uuid128>());
}

TEST(GattTest, characteristic_supports_different_properties)
{
    services::GattService s{ uuid16 };
    services::GattCharacteristicImpl a{ s, uuid16, valueSize, GattPropertyFlags::write | GattPropertyFlags::indicate };
    services::GattCharacteristicImpl b{ s, uuid16, valueSize, GattPropertyFlags::broadcast };

    EXPECT_EQ(GattPropertyFlags::write | GattPropertyFlags::indicate, a.Properties());
    EXPECT_EQ(GattPropertyFlags::broadcast, b.Properties());
}

TEST(GattTest, characteristic_supports_different_permissions)
{
    services::GattService s{ uuid16 };
    services::GattCharacteristicImpl a{ s, uuid16, valueSize, GattPropertyFlags::none, GattPermissionFlags::authorizedRead | GattPermissionFlags::encryptedWrite };
    services::GattCharacteristicImpl b{ s, uuid16, valueSize, GattPropertyFlags::none, GattPermissionFlags::authenticatedRead };

    EXPECT_EQ(GattPermissionFlags::authorizedRead | GattPermissionFlags::encryptedWrite, a.Permissions());
    EXPECT_EQ(GattPermissionFlags::authenticatedRead, b.Permissions());
}

TEST(GattTest, characteristic_is_added_to_service)
{
    services::GattService s{ uuid16 };
    services::GattCharacteristicImpl a{ s, uuid16, valueSize };
    services::GattCharacteristicImpl b{ s, uuid16, valueSize };

    EXPECT_FALSE(s.Characteristics().empty());
    EXPECT_EQ(0x42, s.Characteristics().front().Type().Get<services::GattAttribute::Uuid16>());
}

class GattCharacteristicTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    GattCharacteristicTest()
    {
        characteristic.Attach(operations);
    }

    GattCharacteristicClientOperationsMock operations;
    services::GattService service{ uuid16 };
    services::GattCharacteristicImpl characteristic{ service, uuid16, valueSize };
};

MATCHER_P(ContentsEqual, x, negation ? "Contents not equal" : "Contents are equal")
{
    return infra::ContentsEqual(infra::MakeStringByteRange(x), arg);
}

TEST_F(GattCharacteristicTest, should_update_characteristic_and_callback_on_success)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback);
    EXPECT_CALL(operations, Update(testing::Ref(characteristic), ContentsEqual("string"))).WillOnce(testing::Return(services::GattCharacteristicClientOperations::UpdateStatus::success));
    characteristic.Update(infra::MakeStringByteRange("string"), [&callback]()
        { callback.callback(); });
}

TEST_F(GattCharacteristicTest, should_update_characteristic_and_not_callback_on_error)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(operations, Update(testing::Ref(characteristic), ContentsEqual("string"))).WillOnce(testing::Return(services::GattCharacteristicClientOperations::UpdateStatus::error));
    characteristic.Update(infra::MakeStringByteRange("string"), [&callback]()
        { callback.callback(); });
}

TEST_F(GattCharacteristicTest, should_update_characteristic_and_retry_update_on_retry)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback);
    EXPECT_CALL(operations, Update(testing::Ref(characteristic), ContentsEqual("string"))).WillOnce(testing::Return(services::GattCharacteristicClientOperations::UpdateStatus::retry)).WillOnce(testing::Return(services::GattCharacteristicClientOperations::UpdateStatus::success));
    characteristic.Update(infra::MakeStringByteRange("string"), [&callback]()
        { callback.callback(); });
    ExecuteAllActions();
}
