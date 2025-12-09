
#include "infra/util/BoundedString.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/MemoryRange.hpp"
#include "services/ble/Att.hpp"
#include "services/ble/AttributeStorage.hpp"
#include "gtest/gtest.h"
#include <cstdint>

class AttributeStorageTest
    : public testing::Test
{
public:
    static constexpr std::size_t numberOfEntries = 5;
    static constexpr std::size_t storageSize = 64;
    services::AttributeStorage::WithStorage<numberOfEntries, storageSize> storage;

    std::array<uint8_t, 4> exampleNumberDataRaw = { 0xB, 0xE, 0xE, 0xF };
    infra::ByteRange exampleNumberData{ exampleNumberDataRaw };

    infra::BoundedString::WithStorage<32> exampleStringDataRaw = "fly-you-fools";
    infra::ByteRange exampleStringData{ reinterpret_cast<uint8_t*>(exampleStringDataRaw.begin()), reinterpret_cast<uint8_t*>(exampleStringDataRaw.end()) };
};

TEST_F(AttributeStorageTest, get_non_existing_handle_returns_nullopt)
{
    services::AttAttribute::Handle handle = 1;

    auto data = storage.Get(handle);

    EXPECT_FALSE(data.has_value());
}

TEST_F(AttributeStorageTest, create_valid_handle)
{
    services::AttAttribute::Handle handle = 1;
    std::size_t size = 10;

    storage.Create(handle, size);
}

TEST_F(AttributeStorageTest, create_invalid_handle_abort)
{
    services::AttAttribute::Handle handle = 0;
    std::size_t size = 10;

    EXPECT_DEATH(storage.Create(handle, size), "");
}

TEST_F(AttributeStorageTest, get_created_handle_returns_zeroed_data)
{
    services::AttAttribute::Handle handle = 1;
    std::size_t size = 10;

    storage.Create(handle, size);
    auto data = storage.Get(handle);

    ASSERT_TRUE(data);
    EXPECT_EQ(data->size(), size);
    for (auto byte : data.value())
        EXPECT_EQ(byte, 0);
}

TEST_F(AttributeStorageTest, create_existing_handle_aborts)
{
    services::AttAttribute::Handle handle = 1;
    std::size_t size = 10;

    storage.Create(handle, size);
    EXPECT_DEATH(storage.Create(handle, size), "");
}

TEST_F(AttributeStorageTest, store_non_existing_handle_fails)
{
    services::AttAttribute::Handle handle = 1;

    bool result = storage.Store(handle, exampleNumberData);

    ASSERT_FALSE(result);
}

TEST_F(AttributeStorageTest, get_stored_returns_data)
{
    services::AttAttribute::Handle handle = 1;

    storage.Create(handle, exampleNumberData.size());
    bool result = storage.Store(handle, exampleNumberData);
    ASSERT_TRUE(result);

    auto data = storage.Get(handle);

    ASSERT_TRUE(data);
    EXPECT_NE(data, exampleNumberData); // Different locations in memory
    EXPECT_EQ(data->size(), exampleNumberData.size());
    EXPECT_TRUE(std::equal(data->begin(), data->end(), exampleNumberData.begin()));
}

TEST_F(AttributeStorageTest, store_multiple_get_multiple)
{
    services::AttAttribute::Handle numberHandle = 1;
    services::AttAttribute::Handle stringHandle = 2;

    storage.Create(numberHandle, exampleNumberData.size());
    storage.Create(stringHandle, exampleStringData.size());

    auto result = storage.Store(numberHandle, exampleNumberData);
    ASSERT_TRUE(result);
    result = storage.Store(stringHandle, exampleStringData);
    ASSERT_TRUE(result);

    auto numberData = storage.Get(numberHandle);
    auto stringData = storage.Get(stringHandle);

    ASSERT_TRUE(numberData);
    EXPECT_NE(*numberData, exampleNumberData); // Different locations in memory
    EXPECT_TRUE(std::equal(numberData->begin(), numberData->end(), exampleNumberData.begin()));

    ASSERT_TRUE(stringData);
    EXPECT_NE(*stringData, exampleStringData); // Different locations in memory
    EXPECT_TRUE(std::equal(stringData->begin(), stringData->end(), exampleStringData.begin()));
}

TEST_F(AttributeStorageTest, out_of_storage_memory_abort)
{
    services::AttAttribute::Handle handle = 1;
    services::AttAttribute::Handle otherHandle = 1;

    storage.Create(handle, storageSize);
    EXPECT_DEATH(storage.Create(otherHandle, 1), "");
}

TEST_F(AttributeStorageTest, out_of_entries_memory_abort)
{
    ASSERT_GE(storageSize, numberOfEntries);

    services::AttAttribute::Handle handle = 1;
    for (std::size_t i = 0; i < numberOfEntries; ++i)
        storage.Create(handle++, 1);

    EXPECT_DEATH(storage.Create(handle, 1), "");
}
