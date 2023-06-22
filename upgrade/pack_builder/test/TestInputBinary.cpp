#include "hal/interfaces/test_doubles/FileSystemStub.hpp"
#include "upgrade/pack_builder/ImageEncryptorNone.hpp"
#include "upgrade/pack_builder/InputBinary.hpp"
#include "upgrade/pack_builder/test_helper/ZeroFilledString.hpp"
#include "gtest/gtest.h"
#include <algorithm>

namespace application
{
    struct ImageHeaderNoSecurity
    {
        ImageHeaderPrologue header;
        uint32_t destinationAddress;
        uint32_t binaryLength;
    };
}

class TestInputBinary
    : public testing::Test
{
public:
    TestInputBinary()
        : fileSystem("fileName", std::vector<uint8_t>{ 1, 2, 3, 4 })
        , input("main", "fileName", 4321, fileSystem, encryptor)
    {}

    hal::FileSystemStub fileSystem;
    application::ImageEncryptorNone encryptor;
    application::InputBinary input;
};

TEST_F(TestInputBinary, ImageSize)
{
    std::vector<uint8_t> image = input.Image();

    EXPECT_EQ(28, image.size());
}

TEST_F(TestInputBinary, TargetName)
{
    std::vector<uint8_t> image = input.Image();
    application::ImageHeaderPrologue& header = reinterpret_cast<application::ImageHeaderPrologue&>(image.front());

    EXPECT_EQ(ZeroFilledString(8, "main"), header.targetName.data());
}

TEST_F(TestInputBinary, EncryptionAndMacMethod)
{
    std::vector<uint8_t> image = input.Image();
    application::ImageHeaderPrologue& header = reinterpret_cast<application::ImageHeaderPrologue&>(image.front());

    EXPECT_EQ(0, header.encryptionAndMacMethod);
}

TEST_F(TestInputBinary, DestinationAddress)
{
    std::vector<uint8_t> image = input.Image();
    application::ImageHeaderNoSecurity& header = reinterpret_cast<application::ImageHeaderNoSecurity&>(image.front());

    EXPECT_EQ(4321, header.destinationAddress);
}

TEST_F(TestInputBinary, BinaryLength)
{
    std::vector<uint8_t> image = input.Image();
    application::ImageHeaderNoSecurity& header = reinterpret_cast<application::ImageHeaderNoSecurity&>(image.front());

    EXPECT_EQ(4, header.binaryLength);
}

TEST_F(TestInputBinary, Image)
{
    std::vector<uint8_t> image = input.Image();
    image.erase(image.begin(), image.begin() + sizeof(application::ImageHeaderPrologue) + 8);

    EXPECT_EQ((std::vector<uint8_t>{ 1, 2, 3, 4 }), image);
}

class ImageSecurityStub
    : public application::ImageSecurity
{
public:
    uint32_t EncryptionAndMacMethod() const override
    {
        return 1234;
    }

    std::vector<uint8_t> Secure(const std::vector<uint8_t>& data) const override
    {
        return std::vector<uint8_t>(data.size() / 2, 55);
    }
};

class TestInputBinaryWithEncryptionAndMac
    : public testing::Test
{
public:
    TestInputBinaryWithEncryptionAndMac()
        : fileSystem("fileName", std::vector<uint8_t>{ 1, 2, 3, 4 })
        , input("main", "fileName", 4321, fileSystem, imageSecurity)
    {}

    hal::FileSystemStub fileSystem;
    ImageSecurityStub imageSecurity;
    application::InputBinary input;
};

TEST_F(TestInputBinaryWithEncryptionAndMac, EncryptionAndMacMethod)
{
    std::vector<uint8_t> image = input.Image();
    application::ImageHeaderPrologue& header = reinterpret_cast<application::ImageHeaderPrologue&>(image.front());

    EXPECT_EQ(1234, header.encryptionAndMacMethod);
}

TEST_F(TestInputBinaryWithEncryptionAndMac, Image)
{
    std::vector<uint8_t> image = input.Image();
    image.erase(image.begin(), image.begin() + sizeof(application::ImageHeaderPrologue));

    EXPECT_EQ((std::vector<uint8_t>(6, 55)), image);
}

TEST(TestInputBinaryConstructionWithVector, Construction)
{
    application::ImageEncryptorNone encryptor;
    application::InputBinary input("main", std::vector<uint8_t>{ 1, 2, 3, 4 }, 4321, encryptor);

    std::vector<uint8_t> image = input.Image();
    application::ImageHeaderPrologue& header = reinterpret_cast<application::ImageHeaderPrologue&>(image.front());

    EXPECT_EQ(ZeroFilledString(8, "main"), header.targetName.data());
}
