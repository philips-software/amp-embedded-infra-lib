#include "gmock/gmock.h"
#include "hal/synchronous_interfaces/test_doubles/SynchronousFlashStub.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "upgrade/boot_loader/SecondStageToRamLoader.hpp"
#include "upgrade/pack/UpgradePackHeader.hpp"
#include <algorithm>

class DecryptorSpy
    : public application::Decryptor
{
public:
    virtual infra::ByteRange StateBuffer() override
    {
        return infra::ByteRange();
    }

    virtual void Reset() override
    {}

    virtual void DecryptPart(infra::ByteRange ram) override
    {
        data.resize(ram.size());
        std::copy(ram.begin(), ram.end(), data.begin());
    }

    virtual bool DecryptAndAuthenticate(infra::ByteRange ram) override
    {
        DecryptPart(ram);

        return true;
    }

    std::vector<uint8_t> data;
};

class DecryptorMock
    : public application::Decryptor
{
public:
    virtual infra::ByteRange StateBuffer() override
    {
        return StateBufferMock();
    }

    virtual void Reset() override
    {
        ResetMock();
    }

    virtual void DecryptPart(infra::ByteRange ram) override
    {
        uint32_t first;
        uint32_t second;
        infra::ByteInputStream stream(ram);
        stream >> first >> second;
        return DecryptPartMock(first, second);
    }

    virtual bool DecryptAndAuthenticate(infra::ByteRange ram) override
    {
        return DecryptAndAuthenticateMock(ram);
    }

    MOCK_METHOD0(StateBufferMock, infra::ByteRange());
    MOCK_METHOD0(ResetMock, void());
    MOCK_METHOD2(DecryptPartMock, void(uint32_t, uint32_t));
    MOCK_METHOD1(DecryptAndAuthenticateMock, bool(infra::ByteRange));
};

class VerifierSpy
    : public application::Verifier
{
public:
    virtual bool IsValid(hal::SynchronousFlash& upgradePackFlash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const override
    {
        this->signature.resize(signature.second - signature.first);
        upgradePackFlash.ReadBuffer(this->signature, signature.first);

        this->data.resize(data.second - data.first);
        upgradePackFlash.ReadBuffer(this->data, data.first);

        return true;
    }

    mutable std::vector<uint8_t> signature;
    mutable std::vector<uint8_t> data;
};

class VerifierMock
    : public application::Verifier
{
public:
    virtual bool IsValid(hal::SynchronousFlash& upgradePackFlash, const hal::SynchronousFlash::Range& signature, const hal::SynchronousFlash::Range& data) const override
    {
        return IsValidMock();
    }

    MOCK_CONST_METHOD0(IsValidMock, bool());
};

class SecondStageToRamLoaderTest
    : public testing::Test
{
public:
    SecondStageToRamLoaderTest()
        : upgradePackFlash(1, 512)
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
        std::strncpy(header.epilogue.productName.data(), "test product", header.epilogue.productName.max_size());
        header.epilogue.headerVersion = 1;
        header.epilogue.numberOfImages = numberOfImages;

        return header;
    }

    application::ImageHeaderPrologue CreateImageHeader(const std::string& targetName, std::size_t ExtraSize) const
    {
        application::ImageHeaderPrologue header;
        header.lengthOfHeaderAndImage = sizeof(header) + ExtraSize;
        AssignZeroFilledString(targetName, header.targetName);
        header.encryptionAndMacMethod = 0;

        return header;
    }

public:
    hal::SynchronousFlashStub upgradePackFlash;
    DecryptorSpy decryptorSpy;
    VerifierSpy verifierSpy;
};

TEST_F(SecondStageToRamLoaderTest, NoUpgrade)
{
    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", infra::ByteRange());
    EXPECT_FALSE(secondStageToRamLoader.Load(decryptorSpy, verifierSpy));
}

TEST_F(SecondStageToRamLoaderTest, second_stage_not_loaded_when_ram_is_too_small)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> secondStageImage{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", infra::ByteRange());
    EXPECT_FALSE(secondStageToRamLoader.Load(decryptorSpy, verifierSpy));
    application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<application::UpgradePackHeaderPrologue&>(upgradePackFlash.sectors[0][0]);
    EXPECT_FALSE(static_cast<uint8_t>(prologue.status) & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));
    EXPECT_EQ(application::upgradeErrorCodeNoOrIncorrectSecondStageFound, prologue.errorCode);
}

TEST_F(SecondStageToRamLoaderTest, load_second_stage_when_readyToDeploy)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> secondStageImage{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    std::vector<uint8_t> expectedRam(secondStageImage.begin() + 8, secondStageImage.end());
    std::vector<uint8_t> ram(expectedRam.size(), 0);

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", ram);
    EXPECT_TRUE(secondStageToRamLoader.Load(decryptorSpy, verifierSpy));
    EXPECT_EQ(expectedRam, ram);
}

TEST_F(SecondStageToRamLoaderTest, load_second_stage_when_deployStarted)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));
    header.prologue.status = application::UpgradePackStatus::deployStarted;

    const std::vector<uint8_t> secondStageImage{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    std::vector<uint8_t> expectedRam(secondStageImage.begin() + 8, secondStageImage.end());
    std::vector<uint8_t> ram(expectedRam.size(), 0);

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", ram);
    EXPECT_TRUE(secondStageToRamLoader.Load(decryptorSpy, verifierSpy));
    EXPECT_EQ(expectedRam, ram);
}

TEST_F(SecondStageToRamLoaderTest, dont_load_second_stage_when_not_ready_to_deply)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));
    header.prologue.status = application::UpgradePackStatus::downloaded;

    const std::vector<uint8_t> secondStageImage{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    std::vector<uint8_t> expectedRam(secondStageImage.begin() + 8, secondStageImage.end());
    std::vector<uint8_t> ram(expectedRam.size(), 0);

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", ram);
    EXPECT_FALSE(secondStageToRamLoader.Load(decryptorSpy, verifierSpy));
}

TEST_F(SecondStageToRamLoaderTest, WhenHeaderVersionIncorrectSecondStageDoesNotLoad)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));
    header.epilogue.headerVersion = 2;

    const std::vector<uint8_t> secondStageImage{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    std::vector<uint8_t> expectedRam(secondStageImage.begin() + 8, secondStageImage.end());
    std::vector<uint8_t> ram(expectedRam.size(), 0);

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", ram);
    EXPECT_FALSE(secondStageToRamLoader.Load(decryptorSpy, verifierSpy));

    application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<application::UpgradePackHeaderPrologue&>(upgradePackFlash.sectors[0][0]);
    EXPECT_FALSE(static_cast<uint8_t>(prologue.status) & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));
    EXPECT_EQ(application::upgradeErrorCodeUnknownHeaderVersion, prologue.errorCode);
}

TEST_F(SecondStageToRamLoaderTest, LoadSecondStageAsSecondImage)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(2));

    const std::vector<uint8_t> anotherImage(10, 0);
    application::ImageHeaderPrologue anotherImageHeader(CreateImageHeader("another", anotherImage.size()));

    const std::vector<uint8_t> secondStageImage{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << anotherImageHeader << infra::ConstByteRange(anotherImage) << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    std::vector<uint8_t> expectedRam(secondStageImage.begin() + 8, secondStageImage.end());
    std::vector<uint8_t> ram(expectedRam.size(), 0);

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", ram);
    secondStageToRamLoader.Load(decryptorSpy, verifierSpy);
    EXPECT_EQ(expectedRam, ram);
}

TEST_F(SecondStageToRamLoaderTest, DecryptSecondStage)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> secondStageImage{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    std::vector<uint8_t> expectedRam(secondStageImage.begin() + 8, secondStageImage.end());
    std::vector<uint8_t> ram(expectedRam.size(), 0);

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", ram);
    secondStageToRamLoader.Load(decryptorSpy, verifierSpy);
    EXPECT_EQ(expectedRam, decryptorSpy.data);
}

TEST_F(SecondStageToRamLoaderTest, AuthenticateSecondStage)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> secondStageImage{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    std::vector<uint8_t> ram(secondStageImage.size() - 8, 0);

    testing::StrictMock<DecryptorMock> decryptorMock;
    EXPECT_CALL(decryptorMock, StateBufferMock()).WillOnce(testing::Return(infra::ByteRange()));
    EXPECT_CALL(decryptorMock, ResetMock());
    EXPECT_CALL(decryptorMock, DecryptPartMock(0, 0));
    EXPECT_CALL(decryptorMock, DecryptAndAuthenticateMock(infra::ByteRange(ram))).WillOnce(testing::Return(true));

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", ram);
    EXPECT_TRUE(secondStageToRamLoader.Load(decryptorMock, verifierSpy));
}

TEST_F(SecondStageToRamLoaderTest, InvalidSecondStageDoesNotAuthenticate)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> secondStageImage{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    std::vector<uint8_t> ram(secondStageImage.size() - 8, 0);

    testing::StrictMock<DecryptorMock> decryptorMock;
    EXPECT_CALL(decryptorMock, StateBufferMock()).WillOnce(testing::Return(infra::ByteRange()));
    EXPECT_CALL(decryptorMock, ResetMock());
    EXPECT_CALL(decryptorMock, DecryptPartMock(0, 0));
    EXPECT_CALL(decryptorMock, DecryptAndAuthenticateMock(infra::ByteRange(ram))).WillOnce(testing::Return(false));

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", ram);
    EXPECT_FALSE(secondStageToRamLoader.Load(decryptorMock, verifierSpy));

    application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<application::UpgradePackHeaderPrologue&>(upgradePackFlash.sectors[0][0]);
    EXPECT_FALSE(static_cast<uint8_t>(prologue.status) & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));
    EXPECT_EQ(application::upgradeErrorCodeNoOrIncorrectSecondStageFound, prologue.errorCode);
}

TEST_F(SecondStageToRamLoaderTest, DecryptSecondStageWithState)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> decryptorState = { 1, 2, 3, 4 };
    testing::StrictMock<DecryptorMock> decryptorMock;

    const std::vector<uint8_t> secondStageImage{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size() + decryptorState.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(decryptorState) << infra::ConstByteRange(secondStageImage);

    std::vector<uint8_t> writtenDecryptorState(decryptorState.size(), 0);
    EXPECT_CALL(decryptorMock, StateBufferMock()).WillOnce(testing::Return(infra::ByteRange(writtenDecryptorState)));
    EXPECT_CALL(decryptorMock, ResetMock());

    std::vector<uint8_t> ram(secondStageImage.size() - 8, 0);
    EXPECT_CALL(decryptorMock, DecryptPartMock(0, 0));
    EXPECT_CALL(decryptorMock, DecryptAndAuthenticateMock(infra::ByteRange(ram))).WillOnce(testing::Return(true));

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", ram);
    secondStageToRamLoader.Load(decryptorMock, verifierSpy);
    EXPECT_EQ(decryptorState, writtenDecryptorState);
}

TEST_F(SecondStageToRamLoaderTest, VerifierIsGivenSignature)
{
    UpgradePackHeaderNoSecurity headerNoSecurity(CreateReadyToDeployHeader(1));

    struct UpgradePackHeader
    {
        application::UpgradePackHeaderPrologue prologue;
        uint8_t signature[128];
        application::UpgradePackHeaderEpilogue epilogue;
    } header;

    header.prologue = headerNoSecurity.prologue;
    header.epilogue = headerNoSecurity.epilogue;

    header.prologue.signatureLength = sizeof(header.signature);
    for (uint8_t i = 0; i != sizeof(header.signature); ++i)
        header.signature[i] = i + 1;

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header;

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", infra::ByteRange());
    secondStageToRamLoader.Load(decryptorSpy, verifierSpy);
    EXPECT_TRUE(infra::ContentsEqual(infra::MakeConstByteRange(header.signature), infra::ByteRange(verifierSpy.signature)));
}

TEST_F(SecondStageToRamLoaderTest, VerifierChecksContents)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> secondStageImage{ 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));
    header.prologue.signedContentsLength += sizeof(secondStageImageHeader) + secondStageImage.size();

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", infra::ByteRange());
    secondStageToRamLoader.Load(decryptorSpy, verifierSpy);
    infra::ByteRange signedData = stream.Writer().Processed();
    signedData.pop_front(sizeof(application::UpgradePackHeaderPrologue));
    EXPECT_TRUE(infra::ContentsEqual(signedData, infra::ByteRange(verifierSpy.data)));
}

TEST_F(SecondStageToRamLoaderTest, UpgradeFailsWhenVerificationFails)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));

    const std::vector<uint8_t> secondStageImage{ 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    VerifierMock verifierMock;
    EXPECT_CALL(verifierMock, IsValidMock()).WillOnce(testing::Return(false));

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", infra::ByteRange());
    EXPECT_FALSE(secondStageToRamLoader.Load(decryptorSpy, verifierMock));

    application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<application::UpgradePackHeaderPrologue&>(upgradePackFlash.sectors[0][0]);
    EXPECT_FALSE(static_cast<uint8_t>(prologue.status) & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));
    EXPECT_EQ(application::upgradeErrorCodeInvalidSignature, prologue.errorCode);
}

TEST_F(SecondStageToRamLoaderTest, WhenProductNameIsIncorrectPackIsMarkedAsError)
{
    UpgradePackHeaderNoSecurity header(CreateReadyToDeployHeader(1));
    std::strncpy(header.epilogue.productName.data(), "incorrect product", header.epilogue.productName.max_size());

    const std::vector<uint8_t> secondStageImage{ 1, 1, 2, 3, 5, 8, 13, 21 };
    application::ImageHeaderPrologue secondStageImageHeader(CreateImageHeader("boot2nd", secondStageImage.size()));

    infra::ByteOutputStream stream(upgradePackFlash.sectors[0]);
    stream << header << secondStageImageHeader << infra::ConstByteRange(secondStageImage);

    application::SecondStageToRamLoader secondStageToRamLoader(upgradePackFlash, "test product", infra::ByteRange());
    EXPECT_FALSE(secondStageToRamLoader.Load(decryptorSpy, verifierSpy));
    EXPECT_FALSE(upgradePackFlash.sectors[0][0] & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));

    application::UpgradePackHeaderPrologue& prologue = reinterpret_cast<application::UpgradePackHeaderPrologue&>(upgradePackFlash.sectors[0][0]);
    EXPECT_FALSE(static_cast<uint8_t>(prologue.status) & ~static_cast<uint8_t>(application::UpgradePackStatus::invalid));
    EXPECT_EQ(application::upgradeErrorCodeUnknownProductName, prologue.errorCode);
}
