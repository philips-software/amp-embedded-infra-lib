#include "upgrade/pack_builder/BinaryObject.hpp"
#include "gtest/gtest.h"

TEST(BinaryObjectTest, AddByte)
{
    application::BinaryObject object;
    object.AddHex(std::vector<std::string>{":0100000001fe", ":00000001FF"}, 0, "file");

    application::SparseVector<uint8_t> expectedMemory;
    expectedMemory.Insert(1, 0);
    EXPECT_EQ(expectedMemory, object.Memory());
}

TEST(BinaryObjectTest, AddOtherByte)
{
    application::BinaryObject object;
    object.AddHex(std::vector<std::string>{":0100000002fd", ":00000001FF"}, 0, "file");

    application::SparseVector<uint8_t> expectedMemory;
    expectedMemory.Insert(2, 0);
    EXPECT_EQ(expectedMemory, object.Memory());
}

TEST(BinaryObjectTest, AddMultipleBytes)
{
    application::BinaryObject object;
    object.AddHex(std::vector<std::string>{":020000000102fb", ":00000001FF"}, 0, "file");

    application::SparseVector<uint8_t> expectedMemory;
    expectedMemory.Insert(1, 0);
    expectedMemory.Insert(2, 1);
    EXPECT_EQ(expectedMemory, object.Memory());
}

TEST(BinaryObjectTest, AddMultipleLines)
{
    application::BinaryObject object;
    object.AddHex(std::vector<std::string>{":0100000001fe", ":0100010002fc", ":00000001FF"}, 0, "file");

    application::SparseVector<uint8_t> expectedMemory;
    expectedMemory.Insert(1, 0);
    expectedMemory.Insert(2, 1);
    EXPECT_EQ(expectedMemory, object.Memory());
}

TEST(BinaryObjectTest, ExtendedLinearAddress)
{
    application::BinaryObject object;
    object.AddHex(std::vector<std::string>{":020000040001f9", ":0100000001fe", ":00000001FF"}, 0, "file");

    application::SparseVector<uint8_t> expectedMemory;
    expectedMemory.Insert(1, 0x10000);
    EXPECT_EQ(expectedMemory, object.Memory());
}

TEST(BinaryObjectTest, ExtendedSegmentAddress)
{
    application::BinaryObject object;
    object.AddHex(std::vector<std::string>{":020000021000ec", ":0100000001fe", ":00000001FF"}, 0, "file");

    application::SparseVector<uint8_t> expectedMemory;
    expectedMemory.Insert(1, 0x10000);
    EXPECT_EQ(expectedMemory, object.Memory());
}

TEST(BinaryObjectTest, AddByteAtOffset)
{
    application::BinaryObject object;
    object.AddHex(std::vector<std::string>{":0100000001fe", ":00000001FF"}, 1, "file");

    application::SparseVector<uint8_t> expectedMemory;
    expectedMemory.Insert(1, 1);
    EXPECT_EQ(expectedMemory, object.Memory());
}

TEST(BinaryObjectTest, IgnoreEmptyLines)
{
    application::BinaryObject object;
    object.AddHex(std::vector<std::string>{"", ":00000001FF"}, 0, "file");

    application::SparseVector<uint8_t> expectedMemory;
    EXPECT_EQ(expectedMemory, object.Memory());
}

TEST(BinaryObjectTest, IgnoreStartLinearAddress)
{
    application::BinaryObject object;
    object.AddHex(std::vector<std::string>{":0400000500000000f7", ":00000001FF"}, 0, "file");

    application::SparseVector<uint8_t> expectedMemory;
    EXPECT_EQ(expectedMemory, object.Memory());
}

TEST(BinaryObjectTest, IncorrectCrcThrowsException)
{
    application::BinaryObject object;
    EXPECT_THROW(object.AddHex(std::vector<std::string>{":010000000100", ":00000001FF"}, 0, "file"), application::IncorrectCrcException);
}

TEST(BinaryObjectTest, NoEndOfFileThrowsException)
{
    application::BinaryObject object;
    EXPECT_THROW(object.AddHex(std::vector<std::string>{":0100000001fe"}, 0, "file"), application::NoEndOfFileException);
}

TEST(BinaryObjectTest, DataAfterEndOfFileThrowsException)
{
    application::BinaryObject object;
    EXPECT_THROW(object.AddHex(std::vector<std::string>{":00000001FF", ":0100000001fe"}, 0, "file"), application::DataAfterEndOfFileException);
}

TEST(BinaryObjectTest, UnknownRecordTypeThrowsException)
{
    application::BinaryObject object;
    EXPECT_THROW(object.AddHex(std::vector<std::string>{":0100000e01f0"}, 0, "file"), application::UnknownRecordException);
}

TEST(BinaryObjectTest, TooShortRecord)
{
    application::BinaryObject object;
    EXPECT_THROW(object.AddHex(std::vector<std::string>{":010000"}, 0, "file"), application::RecordTooShortException);
}

TEST(BinaryObjectTest, TooLongRecord)
{
    application::BinaryObject object;
    EXPECT_THROW(object.AddHex(std::vector<std::string>{":0100000001fe0"}, 0, "file"), application::RecordTooLongException);
}
