#include "infra/util/ByteRange.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"
#include "upgrade/pack_builder/InputCommand.hpp"
#include "upgrade/pack_builder/test_helper/ZeroFilledString.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

TEST(TestInputCommand, should_generate_command_image)
{
    application::InputCommand input{ "command" };
    auto image = input.Image();
    auto& header = reinterpret_cast<application::ImageHeaderPrologue&>(image.front());

    EXPECT_EQ(0, header.encryptionAndMacMethod);
    EXPECT_EQ(ZeroFilledString(8, "command"), header.targetName.data());
    EXPECT_EQ(16, image.size());
}

TEST(TestInputCommand, should_generate_command_image_with_blob)
{
    std::array<uint8_t, 8> blob { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    application::InputCommand input{ "cmd-blob", infra::MakeByteRange(blob)};
    auto image = input.Image();
    auto& header = reinterpret_cast<application::ImageHeaderPrologue&>(image.front());

    EXPECT_EQ(0, header.encryptionAndMacMethod);
    EXPECT_EQ(ZeroFilledString(8, "cmd-blob"), header.targetName.data());
    EXPECT_EQ(16+blob.size(), image.size());
    EXPECT_THAT(
        std::vector<uint8_t>(image.end()-blob.size(), image.end()),
        ::testing::ElementsAreArray(blob)
    );
}
