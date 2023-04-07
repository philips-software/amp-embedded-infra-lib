#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/ble/GattServerCharacteristicImpl.hpp"
#include "services/ble/test_doubles/GattServerMock.hpp"
#include "gmock/gmock.h"

namespace
{
    services::AttAttribute::Uuid16 uuid16{ 0x42 };
    services::AttAttribute::Uuid128 uuid128{ { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 } };
    static constexpr uint16_t valueSize = 16;

    using GattPropertyFlags = services::GattCharacteristic::PropertyFlags;
    using GattPermissionFlags = services::GattServerCharacteristic::PermissionFlags;
}

TEST(GattServerTest, characteristic_implementation_handles_are_accesible)
{
    services::GattServerService s{ uuid16 };
    s.Handle() = 0xAB;
    services::GattServerCharacteristicImpl c{ s, uuid16, valueSize };
    c.Handle() = 0xCD;

    EXPECT_EQ(0xAB, c.ServiceHandle());
    EXPECT_EQ(0xCD, c.CharacteristicHandle());
}

TEST(GattServerTest, characteristic_implementation_supports_different_uuid_lengths)
{
    services::GattServerService s{ uuid16 };
    services::GattServerCharacteristicImpl a{ s, uuid16, valueSize };
    services::GattServerCharacteristicImpl b{ s, uuid128, valueSize };

    EXPECT_EQ(0x42, a.Type().Get<services::AttAttribute::Uuid16>());
    EXPECT_EQ((infra::BigEndian<std::array<uint8_t, 16>>{ { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                  0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 } }),
        b.Type().Get<services::AttAttribute::Uuid128>());
}

TEST(GattServerTest, characteristic_implementation_supports_different_properties)
{
    services::GattServerService s{ uuid16 };
    services::GattServerCharacteristicImpl a{ s, uuid16, valueSize, GattPropertyFlags::write | GattPropertyFlags::indicate };
    services::GattServerCharacteristicImpl b{ s, uuid16, valueSize, GattPropertyFlags::broadcast };

    EXPECT_EQ(GattPropertyFlags::write | GattPropertyFlags::indicate, a.Properties());
    EXPECT_EQ(GattPropertyFlags::broadcast, b.Properties());
}

TEST(GattServerTest, characteristic_implementation_supports_different_permissions)
{
    services::GattServerService s{ uuid16 };
    services::GattServerCharacteristicImpl a{ s, uuid16, valueSize, GattPropertyFlags::none, GattPermissionFlags::authorizedRead | GattPermissionFlags::encryptedWrite };
    services::GattServerCharacteristicImpl b{ s, uuid16, valueSize, GattPropertyFlags::none, GattPermissionFlags::authenticatedRead };

    EXPECT_EQ(GattPermissionFlags::authorizedRead | GattPermissionFlags::encryptedWrite, a.Permissions());
    EXPECT_EQ(GattPermissionFlags::authenticatedRead, b.Permissions());
}

TEST(GattServerTest, characteristic_implementation_is_added_to_service)
{
    services::GattServerService s{ uuid16 };
    services::GattServerCharacteristicImpl a{ s, uuid16, valueSize };
    services::GattServerCharacteristicImpl b{ s, uuid16, valueSize };

    EXPECT_FALSE(s.Characteristics().empty());
    EXPECT_EQ(0x42, s.Characteristics().front().Type().Get<services::AttAttribute::Uuid16>());
}

class GattServerCharacteristicTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    GattServerCharacteristicTest()
    {
        characteristic.Attach(operations);
    }

    testing::StrictMock<services::GattServerCharacteristicOperationsMock> operations;
    services::GattServerService service{ uuid16 };
    services::GattServerCharacteristicImpl characteristic{ service, uuid16, valueSize };
};

TEST_F(GattServerCharacteristicTest, should_update_characteristic_and_callback_on_success)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback);
    EXPECT_CALL(operations, Update(testing::Ref(characteristic), infra::ByteRangeContentsEqual(infra::MakeStringByteRange("string")))).WillOnce(testing::Return(services::GattServerCharacteristicOperations::UpdateStatus::success));
    characteristic.Update(infra::MakeStringByteRange("string"), [&callback]() { callback.callback(); });
}

TEST_F(GattServerCharacteristicTest, should_update_characteristic_and_not_callback_on_error)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(operations, Update(testing::Ref(characteristic), infra::ByteRangeContentsEqual(infra::MakeStringByteRange("string")))).WillOnce(testing::Return(services::GattServerCharacteristicOperations::UpdateStatus::error));
    characteristic.Update(infra::MakeStringByteRange("string"), [&callback]() { callback.callback(); });
}

TEST_F(GattServerCharacteristicTest, should_update_characteristic_and_retry_update_on_retry)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback);
    EXPECT_CALL(operations, Update(testing::Ref(characteristic), infra::ByteRangeContentsEqual(infra::MakeStringByteRange("string")))).WillOnce(testing::Return(services::GattServerCharacteristicOperations::UpdateStatus::retry)).WillOnce(testing::Return(services::GattServerCharacteristicOperations::UpdateStatus::success));
    characteristic.Update(infra::MakeStringByteRange("string"), [&callback]() { callback.callback(); });
    ExecuteAllActions();
}
