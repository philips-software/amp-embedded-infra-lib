#include "upgrade/pack/UpgradePackHeader.hpp"
#include "upgrade/pack_builder/InputCommand.hpp"
#include "upgrade/pack_builder/test_helper/ZeroFilledString.hpp"
#include "gtest/gtest.h"

class TestInputCommand
    : public testing::Test
{
public:
    TestInputCommand()
        : input("command")
    {}

    application::InputCommand input;
};

TEST_F(TestInputCommand, should_generate_command_image)
{
    std::vector<uint8_t> image = input.Image();
    application::ImageHeaderPrologue& header = reinterpret_cast<application::ImageHeaderPrologue&>(image.front());

    EXPECT_EQ(0, header.encryptionAndMacMethod);
    EXPECT_EQ(ZeroFilledString(8, "command"), header.targetName.data());
    EXPECT_EQ(16, image.size());
}
