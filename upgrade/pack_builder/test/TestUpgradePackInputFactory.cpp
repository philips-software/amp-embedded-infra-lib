#include "gtest/gtest.h"
#include "hal/interfaces/test_doubles/FileSystemStub.hpp"
#include "infra/util/Function.hpp"
#include "upgrade/pack_builder/ImageEncryptorNone.hpp"
#include "upgrade/pack_builder/UpgradePackInputFactory.hpp"

class TestUpgradePackInputFactory
    : public testing::Test
{
public:
    TestUpgradePackInputFactory()
        : fileSystem("bin_file", std::vector<uint8_t>{})
        , execute([this] {
            fileSystem.WriteFile("hex_file", std::vector<std::string>{ ":020000040001f9", ":0100000001fe", ":00000001FF" });
            fileSystem.WriteBinaryFile("elf_file", std::vector<uint8_t>{ 'E', 'L', 'F', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });

            targets = application::SupportedTargets::Create()
                .AddCmd("cmd")
                .AddBin("bin", 1234)
                .AddHex("hex")
                .AddElf("elf", 5678);
        })
        , factory(fileSystem, targets, encryptor)
    {}

    hal::FileSystemStub fileSystem;
    application::SupportedTargets targets;
    infra::Execute execute;
    application::ImageEncryptorNone encryptor;
    application::UpgradePackInputFactory factory;
};

TEST_F(TestUpgradePackInputFactory, create_Input_for_Command_target)
{
    auto input = factory.CreateInput("cmd", "");
    EXPECT_EQ("cmd", input->TargetName());
}

TEST_F(TestUpgradePackInputFactory, create_Input_for_Bin_target)
{
    auto input = factory.CreateInput("bin", "bin_file");
    EXPECT_EQ("bin", input->TargetName());
}

TEST_F(TestUpgradePackInputFactory, create_Input_for_Hex_target)
{
    auto input = factory.CreateInput("hex", "hex_file");
    EXPECT_EQ("hex", input->TargetName());
}

TEST_F(TestUpgradePackInputFactory, create_Input_for_Elf_target)
{
    auto input = factory.CreateInput("elf", "elf_file");
    EXPECT_EQ("elf", input->TargetName());
}

TEST_F(TestUpgradePackInputFactory, throws_for_unknown_target)
{
    EXPECT_THROW(factory.CreateInput("unknown", ""), application::UnknownTargetException);
}
