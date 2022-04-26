#include "gtest/gtest.h"
#include "upgrade/pack_builder/SupportedTargets.hpp"

TEST(SupportedTargetsTest, should_add_correct_targets)
{
    application::SupportedTargets targets = application::SupportedTargets::Create()
                                                .AddCmd("cmd")
                                                .AddHex("hex")
                                                .AddElf("elf", 1234)
                                                .AddBin("bin", 5678);

    EXPECT_EQ("cmd", targets.CmdTargets()[0]);
    EXPECT_EQ("hex", targets.HexTargets()[0]);
    EXPECT_EQ(application::SupportedTargets::TargetWithOffset{ std::make_pair("elf", 1234) }, targets.ElfTargets()[0]);
    EXPECT_EQ(application::SupportedTargets::TargetWithOffset{ std::make_pair("bin", 5678) }, targets.BinTargets()[0]);
}

TEST(SupportedTargetsTest, should_add_optional_target_by_default)
{
    application::SupportedTargets targets = application::SupportedTargets::Create()
                                                .AddHex("application");

    EXPECT_EQ(0, targets.MandatoryTargets().size());
}

TEST(SupportedTargetsTest, should_add_mandatory_targets)
{
    application::SupportedTargets targets = application::SupportedTargets::Create()
                                                .Mandatory()
                                                .AddHex("application")
                                                .Optional()
                                                .AddHex("data");

    EXPECT_EQ(1, targets.MandatoryTargets().size());
    EXPECT_EQ(2, targets.HexTargets().size());
    EXPECT_EQ("application", targets.MandatoryTargets()[0]);
}
