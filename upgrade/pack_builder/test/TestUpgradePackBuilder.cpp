#include "gtest/gtest.h"
#include "infra/util/Optional.hpp"
#include "upgrade/pack_builder/UpgradePackBuilder.hpp"

class ImageSignerNone
    : public application::ImageSigner
{
public:
    virtual uint16_t SignatureMethod() const override
    {
        return signatureMethod;
    }

    virtual uint16_t SignatureLength() const override
    {
        return 8;
    }

    virtual std::vector<uint8_t> ImageSignature(const std::vector<uint8_t>& upgradePack) override
    {
        return std::vector<uint8_t>(SignatureLength(), 0);
    }

    virtual bool CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image) override
    {
        return checkSignature;
    }

    uint16_t signatureMethod = 0;
    bool checkSignature = true;
};

class InputStub
    : public application::Input
{
public:
    InputStub(const std::vector<uint8_t>& contents)
        : application::Input("stub")
        , contents(contents)
    {}

    virtual std::vector<uint8_t> Image() const override
    {
        return contents;
    }

    std::vector<uint8_t> contents;
};

class TestUpgradePackBuilder
    : public testing::Test
{
public:
    application::UpgradePackBuilder::HeaderInfo headerInfo;
    std::vector<std::unique_ptr<application::Input>> inputs;
    ImageSignerNone signer;
};

TEST_F(TestUpgradePackBuilder, Construction)
{
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
}

TEST_F(TestUpgradePackBuilder, Status)
{
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    const application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<const application::UpgradePackHeaderPrologue&>(upgradePack.front());

    EXPECT_EQ(application::UpgradePackStatus::readyToDeploy, prologue.status);
}

TEST_F(TestUpgradePackBuilder, Magic)
{
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    const application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<const application::UpgradePackHeaderPrologue&>(upgradePack.front());

    EXPECT_EQ(application::upgradePackMagic, prologue.magic);
}

TEST_F(TestUpgradePackBuilder, SignedContentsLengthForNoImages)
{
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    const application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<const application::UpgradePackHeaderPrologue&>(upgradePack.front());

    EXPECT_EQ(204, prologue.signedContentsLength);
}

TEST_F(TestUpgradePackBuilder, SignedContentsLengthForOneImage)
{
    inputs.push_back(std::make_unique<InputStub>(std::vector<uint8_t>(4, 0)));
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    const application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<const application::UpgradePackHeaderPrologue&>(upgradePack.front());

    EXPECT_EQ(208, prologue.signedContentsLength);
}

TEST_F(TestUpgradePackBuilder, SignatureMethod)
{
    signer.signatureMethod = 11;
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    const application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<const application::UpgradePackHeaderPrologue&>(upgradePack.front());

    EXPECT_EQ(11, prologue.signatureMethod);
}

TEST_F(TestUpgradePackBuilder, SignatureLength)
{
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    const application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<const application::UpgradePackHeaderPrologue&>(upgradePack.front());

    EXPECT_EQ(signer.SignatureLength(), prologue.signatureLength);
}

TEST_F(TestUpgradePackBuilder, HeaderVersion)
{
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    const application::UpgradePackHeaderEpilogue& epilogue = reinterpret_cast<const application::UpgradePackHeaderEpilogue&>(*(upgradePack.begin() + sizeof(application::UpgradePackHeaderPrologue)+signer.SignatureLength()));

    EXPECT_EQ(1, epilogue.headerVersion);
}

TEST_F(TestUpgradePackBuilder, check_product_and_component_parts_in_header)
{
    std::array<char, 64> productName = { { 'p', 'r', 'o', 'd' , 'u', 'c', 't', ' ', 'n', 'a', 'm' , 'e' } };
    std::array<char, 64> productVersion = { { 'p', 'r', 'o', 'd' , 'u', 'c', 't', ' ', 'v', 'e', 'r' , 's', 'i', 'o', 'n' } };
    std::array<char, 64> componentName = { { 'c', 'o', 'm', 'p' , 'o', 'n', 'e', 'n', 't', ' ', 'n' , 'a', 'm', 'e' } };

    headerInfo =
    {
        "product name",
        "product version",
        "component name",
        111
    };

    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    const application::UpgradePackHeaderEpilogue& epilogue = reinterpret_cast<const application::UpgradePackHeaderEpilogue&>(*(upgradePack.begin() + sizeof(application::UpgradePackHeaderPrologue) + signer.SignatureLength()));

    EXPECT_EQ(productName, epilogue.productName);
    EXPECT_EQ(productVersion, epilogue.productVersion);
    EXPECT_EQ(componentName, epilogue.componentName);
    EXPECT_EQ(111, epilogue.componentVersion);
}

TEST_F(TestUpgradePackBuilder, HeaderLength)
{
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    const application::UpgradePackHeaderEpilogue& epilogue = reinterpret_cast<const application::UpgradePackHeaderEpilogue&>(*(upgradePack.begin() + sizeof(application::UpgradePackHeaderPrologue) + signer.SignatureLength()));

    EXPECT_EQ(sizeof(application::UpgradePackHeaderPrologue) + signer.SignatureLength() + sizeof(application::UpgradePackHeaderEpilogue), epilogue.headerLength);
}

TEST_F(TestUpgradePackBuilder, NoImages)
{
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    const application::UpgradePackHeaderEpilogue& epilogue = reinterpret_cast<const application::UpgradePackHeaderEpilogue&>(*(upgradePack.begin() + sizeof(application::UpgradePackHeaderPrologue)+signer.SignatureLength()));

    EXPECT_EQ(0, epilogue.numberOfImages);
}

TEST_F(TestUpgradePackBuilder, OneImage)
{
    inputs.push_back(std::make_unique<InputStub>(std::vector<uint8_t>(4, 0)));
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    const application::UpgradePackHeaderEpilogue& epilogue = reinterpret_cast<const application::UpgradePackHeaderEpilogue&>(*(upgradePack.begin() + sizeof(application::UpgradePackHeaderPrologue) + signer.SignatureLength()));

    EXPECT_EQ(1, epilogue.numberOfImages);
}

TEST_F(TestUpgradePackBuilder, ImageContents)
{
    inputs.push_back(std::make_unique<InputStub>(std::vector<uint8_t>{1, 2, 3, 4}));
    application::UpgradePackBuilder upgradePackBuilder(headerInfo, std::move(inputs), signer);
    std::vector<uint8_t> upgradePack = upgradePackBuilder.UpgradePack();
    std::vector<uint8_t> imageContents(upgradePack.begin() + sizeof(application::UpgradePackHeaderPrologue) + signer.SignatureLength() + sizeof(application::UpgradePackHeaderEpilogue), upgradePack.end());

    EXPECT_EQ((std::vector<uint8_t>{1, 2, 3, 4}), imageContents);
}

TEST_F(TestUpgradePackBuilder, SignatureDoesNotVerify)
{
    signer.checkSignature = false;
    infra::Optional<application::UpgradePackBuilder> upgradePackBuilder;
    EXPECT_THROW(upgradePackBuilder.Emplace(headerInfo, std::move(inputs), signer), application::SignatureDoesNotVerifyException);
}
