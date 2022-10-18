#include "hal/synchronous_interfaces/test_doubles/SynchronousFlashStub.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "upgrade/boot_loader/DecryptorNone.hpp"
#include "upgrade/boot_loader/PackUpgrader.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"
#include "gmock/gmock.h"

class ImageUpgraderMock
    : public application::ImageUpgrader
{
public:
    ImageUpgraderMock()
        : application::ImageUpgrader("upgrader", decryptorNone)
    {}

    virtual uint32_t Upgrade(hal::SynchronousFlash& flash, uint32_t imageAddress, uint32_t imageSize, uint32_t destinationAddress) override
    {
        return UpgradeMock(imageAddress, imageSize, destinationAddress);
    }

    MOCK_METHOD3(UpgradeMock, uint32_t(uint32_t, uint32_t, uint32_t));

    application::DecryptorNone decryptorNone;
};

class PackUpgraderTest
    : public testing::Test
{
public:
    PackUpgraderTest()
        : upgradePackFlash(1, 4096)
        , singleUpgraderMock({ &imageUpgraderMock })
    {}

    void AssignZeroFilledString(const std::string& from, infra::MemoryRange<char> to) const
    {
        std::copy(from.begin(), from.begin() + std::min(from.size(), to.size()), to.begin());
        std::fill(to.begin() + std::min(from.size(), to.size()), to.end(), 0);
    }

    struct UpgradePackHeaderNoSecurity
    {
        application::UpgradePackHeaderPrologue prologue;
        application::UpgradePackHeaderEpilogue epilogue;
    };

    UpgradePackHeaderNoSecurity CreateReadyToDeployHeader(std::size_t numberOfImages) const
    {
        UpgradePackHeaderNoSecurity header = {};
        header.prologue.status = application::UpgradePackStatus::readyToDeploy;
        header.prologue.magic = application::upgradePackMagic;
        header.prologue.errorCode = 0xffffffff;
        header.prologue.signedContentsLength = sizeof(application::UpgradePackHeaderEpilogue);
        header.epilogue.headerVersion = 1;
        header.epilogue.numberOfImages = numberOfImages;

        return header;
    }

    application::ImageHeaderPrologue CreateImageHeaderPrologue(const std::string& targetName, std::size_t ExtraSize) const
    {
        application::ImageHeaderPrologue header;
        header.lengthOfHeaderAndImage = sizeof(header) + sizeof(application::ImageHeaderEpilogue) + ExtraSize;
        AssignZeroFilledString(targetName, header.targetName);
        header.encryptionAndMacMethod = 0;

        return header;
    }

    application::ImageHeaderEpilogue CreateImageHeaderEpilogue() const
    {
        application::ImageHeaderEpilogue header{ 1, 2 };

        return header;
    }

public:
    hal::SynchronousFlashStub upgradePackFlash;
    testing::StrictMock<ImageUpgraderMock> imageUpgraderMock;
    std::array<application::ImageUpgrader*, 1> singleUpgraderMock;
};

TEST_F(PackUpgraderTest, Construction)
{
    application::PackUpgrader upgrader(upgradePackFlash);
}

TEST_F(PackUpgraderTest, ImageInvokesImageUpgrader)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> image{ 1, 5 };
    application::ImageHeaderPrologue imageHeaderPrologue(CreateImageHeaderPrologue("upgrader", image.size()));
    application::ImageHeaderEpilogue imageHeaderEpilogue = CreateImageHeaderEpilogue();

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << imageHeaderPrologue << imageHeaderEpilogue << infra::ConstByteRange(image);

    EXPECT_CALL(imageUpgraderMock, UpgradeMock(244, 2, 1)).WillOnce(testing::Return(0));
    application::PackUpgrader packUpgrader(upgradePackFlash);
    packUpgrader.UpgradeFromImages(singleUpgraderMock);
}

TEST_F(PackUpgraderTest, PackIsMarkedAsDeployed)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> image{ 1, 5 };
    application::ImageHeaderPrologue imageHeaderPrologue(CreateImageHeaderPrologue("upgrader", image.size()));
    application::ImageHeaderEpilogue imageHeaderEpilogue = CreateImageHeaderEpilogue();

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << imageHeaderPrologue << imageHeaderEpilogue << infra::ConstByteRange(image);

    EXPECT_CALL(imageUpgraderMock, UpgradeMock(244, 2, 1)).WillOnce(testing::Return(0));
    application::PackUpgrader packUpgrader(upgradePackFlash);
    packUpgrader.UpgradeFromImages(singleUpgraderMock);

    EXPECT_EQ(application::UpgradePackStatus::deployed, static_cast<application::UpgradePackStatus>(upgradePackFlash.sectors[0][0]));
}

TEST_F(PackUpgraderTest, when_upgrade_starts_pack_is_marked_as_deployStarted)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> image{ 1, 5 };
    application::ImageHeaderPrologue imageHeaderPrologue(CreateImageHeaderPrologue("upgrader", image.size()));
    application::ImageHeaderEpilogue imageHeaderEpilogue = CreateImageHeaderEpilogue();

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << imageHeaderPrologue << imageHeaderEpilogue << infra::ConstByteRange(image);

    EXPECT_CALL(imageUpgraderMock, UpgradeMock(244, 2, 1)).WillOnce(testing::Throw<int>(0));
    application::PackUpgrader packUpgrader(upgradePackFlash);
    EXPECT_THROW(packUpgrader.UpgradeFromImages(singleUpgraderMock), int);

    EXPECT_EQ(application::UpgradePackStatus::deployStarted, static_cast<application::UpgradePackStatus>(upgradePackFlash.sectors[0][0]));
}

TEST_F(PackUpgraderTest, pack_that_was_already_started_will_be_retried)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));
    header.prologue.status = application::UpgradePackStatus::deployStarted;

    const std::vector<uint8_t> image{ 1, 5 };
    application::ImageHeaderPrologue imageHeaderPrologue(CreateImageHeaderPrologue("upgrader", image.size()));
    application::ImageHeaderEpilogue imageHeaderEpilogue = CreateImageHeaderEpilogue();

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << imageHeaderPrologue << imageHeaderEpilogue << infra::ConstByteRange(image);

    EXPECT_CALL(imageUpgraderMock, UpgradeMock(244, 2, 1)).WillOnce(testing::Return(0));
    application::PackUpgrader packUpgrader(upgradePackFlash);
    packUpgrader.UpgradeFromImages(singleUpgraderMock);

    EXPECT_EQ(application::UpgradePackStatus::deployed, static_cast<application::UpgradePackStatus>(upgradePackFlash.sectors[0][0]));
}

TEST_F(PackUpgraderTest, WhenVersionIsIncorrectPackIsMarkedAsError)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));
    header.epilogue.headerVersion = 2;

    const std::vector<uint8_t> image{ 1, 5 };
    application::ImageHeaderPrologue imageHeaderPrologue(CreateImageHeaderPrologue("upgrader", image.size()));
    application::ImageHeaderEpilogue imageHeaderEpilogue = CreateImageHeaderEpilogue();

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << imageHeaderPrologue << imageHeaderEpilogue << infra::ConstByteRange(image);

    application::PackUpgrader packUpgrader(upgradePackFlash);
    packUpgrader.UpgradeFromImages(singleUpgraderMock);

    EXPECT_FALSE(upgradePackFlash.sectors[0][0] & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));

    application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<application::UpgradePackHeaderPrologue&>(upgradePackFlash.sectors[0][0]);
    EXPECT_FALSE(static_cast<uint8_t>(prologue.status) & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));
    EXPECT_EQ(application::upgradeErrorCodeUnknownHeaderVersion, prologue.errorCode);
}

TEST_F(PackUpgraderTest, WhenUpgraderIsNotFoundPackIsMarkedAsError)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> image{ 1, 5 };
    application::ImageHeaderPrologue imageHeaderPrologue(CreateImageHeaderPrologue("unknown", image.size()));
    application::ImageHeaderEpilogue imageHeaderEpilogue = CreateImageHeaderEpilogue();

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << imageHeaderPrologue << imageHeaderEpilogue << infra::ConstByteRange(image);

    application::PackUpgrader packUpgrader(upgradePackFlash);
    packUpgrader.UpgradeFromImages(singleUpgraderMock);

    EXPECT_FALSE(upgradePackFlash.sectors[0][0] & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));

    application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<application::UpgradePackHeaderPrologue&>(upgradePackFlash.sectors[0][0]);
    EXPECT_FALSE(static_cast<uint8_t>(prologue.status) & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));
    EXPECT_EQ(application::upgradeErrorCodeNoSuitableImageUpgraderFound, prologue.errorCode);
}

TEST_F(PackUpgraderTest, WhenUpgraderCannotUpgradePackIsMarkedAsError)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> image{ 1, 5 };
    application::ImageHeaderPrologue imageHeaderPrologue(CreateImageHeaderPrologue("upgrader", image.size()));
    application::ImageHeaderEpilogue imageHeaderEpilogue = CreateImageHeaderEpilogue();

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << imageHeaderPrologue << imageHeaderEpilogue << infra::ConstByteRange(image);

    EXPECT_CALL(imageUpgraderMock, UpgradeMock(244, 2, 1)).WillOnce(testing::Return(application::upgradeErrorCodeImageUpgradeFailed));
    application::PackUpgrader packUpgrader(upgradePackFlash);
    packUpgrader.UpgradeFromImages(singleUpgraderMock);

    EXPECT_FALSE(upgradePackFlash.sectors[0][0] & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));

    application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<application::UpgradePackHeaderPrologue&>(upgradePackFlash.sectors[0][0]);
    EXPECT_FALSE(static_cast<uint8_t>(prologue.status) & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));
    EXPECT_EQ(application::upgradeErrorCodeImageUpgradeFailed, prologue.errorCode);
}

TEST_F(PackUpgraderTest, HasImage_finds_image)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> image{ 1, 5 };
    application::ImageHeaderPrologue imageHeaderPrologue(CreateImageHeaderPrologue("image", image.size()));
    application::ImageHeaderEpilogue imageHeaderEpilogue = CreateImageHeaderEpilogue();

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << imageHeaderPrologue << imageHeaderEpilogue << infra::ConstByteRange(image);

    application::PackUpgrader packUpgrader(upgradePackFlash);
    EXPECT_TRUE(packUpgrader.HasImage("image"));
    EXPECT_FALSE(packUpgrader.HasImage("nomage"));
}

TEST_F(PackUpgraderTest, HasImage_finds_second_image)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(2));

    const std::vector<uint8_t> image{ 1, 5 };
    application::ImageHeaderPrologue imageHeaderPrologue(CreateImageHeaderPrologue("image", image.size()));
    application::ImageHeaderEpilogue imageHeaderEpilogue = CreateImageHeaderEpilogue();
    application::ImageHeaderPrologue imageHeader2Prologue(CreateImageHeaderPrologue("2image", image.size()));
    application::ImageHeaderEpilogue imageHeader2Epilogue = CreateImageHeaderEpilogue();

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << imageHeaderPrologue << imageHeaderEpilogue << infra::ConstByteRange(image) << imageHeader2Prologue << imageHeader2Epilogue << infra::ConstByteRange(image);

    application::PackUpgrader packUpgrader(upgradePackFlash);
    EXPECT_TRUE(packUpgrader.HasImage("2image"));
}
