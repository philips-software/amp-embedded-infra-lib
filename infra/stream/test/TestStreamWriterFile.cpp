#include "infra/stream/StreamErrorPolicy.hpp"
#include "infra/stream/StreamWriterFile.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/ByteRange.hpp"
#include "gmock/gmock.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

namespace
{
    constexpr auto file = "StreamWriterFile.out";
}

class StreamWriterFileTest
    : public testing::Test
{
public:
    ~StreamWriterFileTest()
    {
        std::filesystem::remove(file);
    }

    infra::StreamErrorPolicy noFailPolicy{ infra::noFail };
};

TEST_F(StreamWriterFileTest, WriteString)
{
    {
        infra::StreamWriterFile streamWriter{ file };
        streamWriter.Insert(infra::StdStringAsByteRange("Hello, world!"), noFailPolicy);
    }

    std::ifstream fileStream{ file };
    std::string content{ std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>() };
    EXPECT_THAT(content, testing::StrEq("Hello, world!"));
}

TEST_F(StreamWriterFileTest, WriteDecimal)
{
    {
        infra::StreamWriterFile streamWriter{ file };
        streamWriter.Insert(infra::MakeByteRange(std::uint64_t{ 0xCCCCCCCCCCCCCCCC }), noFailPolicy);
    }

    std::ifstream fileStream{ file };
    std::uint64_t actual{ 0xFFFFFFFFFFFFFFFF };
    fileStream.read(reinterpret_cast<char*>(&actual), sizeof(actual));
    EXPECT_THAT(fileStream.fail(), testing::Eq(false));
    EXPECT_THAT(actual, testing::Eq(0xCCCCCCCCCCCCCCCC));
}

TEST_F(StreamWriterFileTest, OverwriteExistingFile)
{
    {
        infra::StreamWriterFile streamWriter{ file };
        streamWriter.Insert(infra::StdStringAsByteRange("Hello"), noFailPolicy);
    }

    {
        infra::StreamWriterFile streamWriter{ file };
        streamWriter.Insert(infra::StdStringAsByteRange("world!"), noFailPolicy);
    }

    std::ifstream fileStream{ file };
    std::string content{ std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>() };
    EXPECT_THAT(content, testing::StrEq("world!"));
}
