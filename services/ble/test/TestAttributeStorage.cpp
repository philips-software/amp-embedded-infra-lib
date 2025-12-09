
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
    static constexpr std::size_t numberOfEntries = 4;
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

    auto data = storage.Read(handle);

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

TEST_F(AttributeStorageTest, get_created_empty_returns_empty)
{
    services::AttAttribute::Handle handle = 1;
    std::size_t size = 10;

    storage.Create(handle, size);
    auto data = storage.Read(handle);

    ASSERT_TRUE(data);
    EXPECT_EQ(data->size(), 0);
}

TEST_F(AttributeStorageTest, create_existing_handle_aborts)
{
    services::AttAttribute::Handle handle = 1;
    std::size_t size = 10;

    storage.Create(handle, size);
    EXPECT_DEATH(storage.Create(handle, size), "");
}

TEST_F(AttributeStorageTest, created_data_always_aligned)
{
    services::AttAttribute::Handle handle = 1;
    services::AttAttribute::Handle otherHandle = 2;
    std::size_t size = 1;

    storage.Create(handle, size);
    auto data = storage.Read(handle);
    ASSERT_TRUE(data);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(data->begin()) % alignof(std::max_align_t), 0u);

    storage.Create(otherHandle, size);
    auto otherData = storage.Read(otherHandle);
    ASSERT_TRUE(otherData);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(otherData->begin()) % alignof(std::max_align_t), 0u);
}

TEST_F(AttributeStorageTest, Write_non_existing_handle_fails)
{
    services::AttAttribute::Handle handle = 1;

    bool result = storage.Write(handle, exampleNumberData);

    ASSERT_FALSE(result);
}

TEST_F(AttributeStorageTest, get_written_returns_data)
{
    services::AttAttribute::Handle handle = 1;

    storage.Create(handle, exampleNumberData.size());
    bool result = storage.Write(handle, exampleNumberData);
    ASSERT_TRUE(result);

    auto data = storage.Read(handle);

    ASSERT_TRUE(data);
    EXPECT_NE(data, exampleNumberData); // Different locations in memory
    EXPECT_EQ(data->size(), exampleNumberData.size());
    EXPECT_TRUE(std::equal(data->begin(), data->end(), exampleNumberData.begin()));
}

TEST_F(AttributeStorageTest, get_written_bigger_allocation_returns_only_written_data)
{
    services::AttAttribute::Handle handle = 1;
    std::size_t biggerValueSize = exampleNumberData.size() + 32;

    storage.Create(handle, biggerValueSize);
    bool result = storage.Write(handle, exampleNumberData);
    ASSERT_TRUE(result);

    auto data = storage.Read(handle);

    ASSERT_TRUE(data);
    EXPECT_NE(data, exampleNumberData); // Different locations in memory
    EXPECT_EQ(data->size(), exampleNumberData.size());
    EXPECT_NE(data->size(), biggerValueSize);
    EXPECT_TRUE(std::equal(data->begin(), data->end(), exampleNumberData.begin()));
}

TEST_F(AttributeStorageTest, Write_multiple_get_multiple)
{
    services::AttAttribute::Handle numberHandle = 1;
    services::AttAttribute::Handle stringHandle = 2;

    storage.Create(numberHandle, exampleNumberData.size());
    storage.Create(stringHandle, exampleStringData.size());

    auto result = storage.Write(numberHandle, exampleNumberData);
    ASSERT_TRUE(result);
    result = storage.Write(stringHandle, exampleStringData);
    ASSERT_TRUE(result);

    auto numberData = storage.Read(numberHandle);
    auto stringData = storage.Read(stringHandle);

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
    ASSERT_GE(storageSize, numberOfEntries * 4); // Compensate for alignment overhead

    services::AttAttribute::Handle handle = 1;
    for (std::size_t i = 0; i < numberOfEntries; ++i)
        storage.Create(handle++, 1);

    EXPECT_DEATH(storage.Create(handle, 1), "");
}
