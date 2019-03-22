#include "gtest/gtest.h"
#include "infra/syntax/JsonInputStream.hpp"

class JsonInputStreamTest
    : public testing::Test
{
public:
    JsonInputStreamTest()
        : errorPolicy(infra::noFail)
        , string(R"(ab\")")
        , reader(string)
    {}

    infra::StreamErrorPolicy errorPolicy;
    infra::JsonString string;
    infra::JsonInputStreamReader reader;
};

TEST_F(JsonInputStreamTest, read_JsonString)
{
    infra::JsonInputStream stream(string);

    char a;
    char b;
    char quote;
    stream >> a >> b >> quote;

    EXPECT_EQ('a', a);
    EXPECT_EQ('b', b);
    EXPECT_EQ('"', quote);
}

TEST_F(JsonInputStreamTest, Extract)
{
    std::array<uint8_t, 3> result;
    reader.Extract(result, errorPolicy);
    EXPECT_EQ((std::array<uint8_t, 3>{ 'a', 'b', '"' }), result);
}

TEST_F(JsonInputStreamTest, Peek)
{
    EXPECT_EQ('a', reader.Peek(errorPolicy));
    EXPECT_FALSE(errorPolicy.Failed());

    std::array<uint8_t, 3> result;
    reader.Extract(result, errorPolicy);

    EXPECT_EQ(0, reader.Peek(errorPolicy));
    EXPECT_TRUE(errorPolicy.Failed());
}

TEST_F(JsonInputStreamTest, Empty)
{
    EXPECT_FALSE(reader.Empty());

    std::array<uint8_t, 3> result;
    reader.Extract(result, errorPolicy);

    EXPECT_TRUE(reader.Empty());
}

TEST_F(JsonInputStreamTest, Available)
{
    EXPECT_EQ(3, reader.Available());

    std::array<uint8_t, 3> result;
    reader.Extract(result, errorPolicy);

    EXPECT_EQ(0, reader.Available());
}
