#include "generated/echo/UpgradeKeys.pb.hpp"
#include "hal/generic/SynchronousRandomDataGeneratorGeneric.hpp"
#include "hal/interfaces/test_doubles/FileSystemStub.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "infra/util/MemoryRange.hpp"
#include "upgrade/pack_builder/ImageSignerEcDsa.hpp"
#include "upgrade/security_key_generator/material_generator/MaterialGenerator.hpp"
#include "gtest/gtest.h"
#include <vector>

class MaterialGeneratorTest
    : public testing::Test
{
public:
    hal::SynchronousRandomDataGeneratorGeneric randomDataGenerator;
    hal::FileSystemStub fileSystem;
    application::MaterialGenerator materialGenerator{ randomDataGenerator, fileSystem };

    std::string GetRawKey(const std::vector<std::string> contents, const std::string keyName)
    {
        auto foundKey = false;
        std::string rawKey;

        for (auto line : contents)
        {
            if (foundKey)
                rawKey += line;
            else if (line.find(keyName) != std::string::npos)
            {
                foundKey = true;
                rawKey = line;
            }

            if (foundKey && rawKey.find("};") != std::string::npos)
                break;
        }

        if (!foundKey)
            throw std::runtime_error("Key: " + keyName + " is not found.");

        return rawKey;
    }

    std::vector<uint8_t> ExtractKey(std::string rawKey)
    {
        auto innerOpen = rawKey.find_last_of('{');
        auto innerClose = rawKey.find_first_of('}');
        rawKey = rawKey.substr(innerOpen + 1, innerClose - innerOpen - 1);

        std::vector<uint8_t> extractedValues;

        while (rawKey.find(',') != std::string::npos)
        {
            extractedValues.push_back(std::stoi(rawKey, nullptr, 16));
            rawKey = rawKey.substr(rawKey.find(',') + 1);
        }

        extractedValues.push_back(std::stoi(rawKey, nullptr, 16));

        return extractedValues;
    }
};

TEST_F(MaterialGeneratorTest, generate_and_write_224_key)
{
    materialGenerator.GenerateKeys(application::MaterialGenerator::EcDsaKey::ecDsa224);
    materialGenerator.WriteKeys("key224.cpp");
    materialGenerator.WriteKeysProto("key224.bin");

    auto keysCpp = fileSystem.ReadFile("key224.cpp");
    auto aesKeyCpp = ExtractKey(GetRawKey(keysCpp, "aesKey"));
    auto publicKeyCpp = ExtractKey(GetRawKey(keysCpp, "ecDsaPublicKey"));
    auto privateKeyCpp = ExtractKey(GetRawKey(keysCpp, "ecDsaPrivateKey"));

    auto upgradeKeysProto = fileSystem.ReadBinaryFile("key224.bin");
    infra::ByteInputStream stream(upgradeKeysProto);
    infra::ProtoParser protoParser(stream);
    upgrade_keys::keys224 upgradeKeys(protoParser);
    infra::ConstByteRange aeskeyBin(upgradeKeys.aesKey.symmetricKey.begin(), upgradeKeys.aesKey.symmetricKey.end());
    infra::ConstByteRange publicKeyBin(upgradeKeys.ecDsaKey.publicKey.begin(), upgradeKeys.ecDsaKey.publicKey.end());
    infra::ConstByteRange privateKeyBin(upgradeKeys.ecDsaKey.privateKey.begin(), upgradeKeys.ecDsaKey.privateKey.end());

    EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(aesKeyCpp), aeskeyBin));
    EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(publicKeyCpp), publicKeyBin));
    EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(privateKeyCpp), privateKeyBin));

    EXPECT_EQ(16, aesKeyCpp.size());
    EXPECT_EQ(56, publicKeyCpp.size());
    EXPECT_EQ(56, privateKeyCpp.size());

    application::ImageSignerEcDsa224 imageSinger{ randomDataGenerator, publicKeyCpp, privateKeyCpp };
    auto sign = imageSinger.ImageSignature({ 1, 2, 3, 4 });
    EXPECT_TRUE(imageSinger.CheckSignature(sign, { 1, 2, 3, 4 }));
}

TEST_F(MaterialGeneratorTest, generate_and_write_256_key)
{
    materialGenerator.GenerateKeys(application::MaterialGenerator::EcDsaKey::ecDsa256);
    materialGenerator.WriteKeys("key256.cpp");
    materialGenerator.WriteKeysProto("key256.bin");

    auto keysCpp = fileSystem.ReadFile("key256.cpp");
    auto aesKeyCpp = ExtractKey(GetRawKey(keysCpp, "aesKey"));
    auto publicKeyCpp = ExtractKey(GetRawKey(keysCpp, "ecDsaPublicKey"));
    auto privateKeyCpp = ExtractKey(GetRawKey(keysCpp, "ecDsaPrivateKey"));

    auto upgradeKeysProto = fileSystem.ReadBinaryFile("key256.bin");
    infra::ByteInputStream stream(upgradeKeysProto);
    infra::ProtoParser protoParser(stream);
    upgrade_keys::keys256 upgradeKeys(protoParser);
    infra::ConstByteRange aeskeyBin(upgradeKeys.aesKey.symmetricKey.begin(), upgradeKeys.aesKey.symmetricKey.end());
    infra::ConstByteRange publicKeyBin(upgradeKeys.ecDsaKey.publicKey.begin(), upgradeKeys.ecDsaKey.publicKey.end());
    infra::ConstByteRange privateKeyBin(upgradeKeys.ecDsaKey.privateKey.begin(), upgradeKeys.ecDsaKey.privateKey.end());

    EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(aesKeyCpp), aeskeyBin));
    EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(publicKeyCpp), publicKeyBin));
    EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(privateKeyCpp), privateKeyBin));

    EXPECT_EQ(16, aesKeyCpp.size());
    EXPECT_EQ(64, publicKeyCpp.size());
    EXPECT_EQ(64, privateKeyCpp.size());

    application::ImageSignerEcDsa256 imageSinger{ randomDataGenerator, publicKeyCpp, privateKeyCpp };
    auto sign = imageSinger.ImageSignature({ 1, 2, 3, 4 });
    EXPECT_TRUE(imageSinger.CheckSignature(sign, { 1, 2, 3, 4 }));
}

TEST_F(MaterialGeneratorTest, convert_key_256)
{
    std::vector<std::string> keyFile{ "const std::array<uint8_t, 16> aesKey = { { 0xf9, 0xe4, 0x39, 0x1f, 0x60, 0x80, 0x98, 0x85,",
        "    0x88, 0x6d, 0x44, 0x88, 0xc5, 0xf4, 0xb9, 0xf5 } };",
        "const std::array<uint8_t, 64> ecDsaPublicKey = { { 0x3e, 0x6b, 0xe0, 0x8c, 0x64, 0x3e, 0xf8, 0x26,",
        "    0x3f, 0x9c, 0xdc, 0x7e, 0x75, 0x5d, 0x17, 0x04,",
        "    0xf2, 0xfd, 0xe0, 0xf4, 0x0f, 0x35, 0xc3, 0x01,",
        "    0x9d, 0x8b, 0x2b, 0xb1, 0xd2, 0x87, 0xdb, 0x5e,",
        "    0x92, 0x9e, 0x87, 0x66, 0x59, 0x44, 0xb5, 0xb7,",
        "    0xa5, 0xab, 0x7f, 0xbf, 0xe7, 0x8c, 0x8f, 0xee,",
        "    0x9d, 0xa2, 0x67, 0xf2, 0x7a, 0x27, 0xb4, 0x56,",
        "    0x92, 0x11, 0x90, 0x2e, 0x54, 0x64, 0x31, 0x4c } };",
        "const std::array<uint8_t, 64> ecDsaPrivateKey = { { 0xff, 0xd1, 0x3b, 0x3b, 0x7f, 0xb7, 0xf0, 0xd4,",
        "    0x60, 0x29, 0x27, 0x0d, 0xc5, 0x7e, 0xa6, 0x75,",
        "    0x2a, 0xbb, 0x2c, 0x98, 0xe9, 0xe2, 0x4c, 0xfa,",
        "    0xb6, 0x34, 0x0a, 0xcb, 0x45, 0x99, 0xc0, 0x61,",
        "    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,",
        "    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,",
        "    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,",
        "    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };};" };
    fileSystem.WriteFile("key256.cpp", keyFile);

    materialGenerator.ImportKeys("key256.cpp");
    materialGenerator.WriteKeysProto("key256.bin");
    auto upgradeKeysProto = fileSystem.ReadBinaryFile("key256.bin");
    infra::ByteInputStream stream(upgradeKeysProto);
    infra::ProtoParser protoParser(stream);
    upgrade_keys::keys256 upgradeKeys(protoParser);
    infra::ConstByteRange aeskeyBin(upgradeKeys.aesKey.symmetricKey.begin(), upgradeKeys.aesKey.symmetricKey.end());
    infra::ConstByteRange publicKeyBin(upgradeKeys.ecDsaKey.publicKey.begin(), upgradeKeys.ecDsaKey.publicKey.end());
    infra::ConstByteRange privateKeyBin(upgradeKeys.ecDsaKey.privateKey.begin(), upgradeKeys.ecDsaKey.privateKey.end());

    EXPECT_TRUE(infra::ContentsEqual(aeskeyBin, infra::MakeRange({ 0xf9, 0xe4, 0x39, 0x1f, 0x60, 0x80, 0x98, 0x85, 0x88, 0x6d, 0x44, 0x88, 0xc5, 0xf4, 0xb9, 0xf5 })));
    EXPECT_TRUE(infra::ContentsEqual(publicKeyBin, infra::MakeRange({ 0x3e, 0x6b, 0xe0, 0x8c, 0x64, 0x3e, 0xf8, 0x26,
                                                       0x3f, 0x9c, 0xdc, 0x7e, 0x75, 0x5d, 0x17, 0x04,
                                                       0xf2, 0xfd, 0xe0, 0xf4, 0x0f, 0x35, 0xc3, 0x01,
                                                       0x9d, 0x8b, 0x2b, 0xb1, 0xd2, 0x87, 0xdb, 0x5e,
                                                       0x92, 0x9e, 0x87, 0x66, 0x59, 0x44, 0xb5, 0xb7,
                                                       0xa5, 0xab, 0x7f, 0xbf, 0xe7, 0x8c, 0x8f, 0xee,
                                                       0x9d, 0xa2, 0x67, 0xf2, 0x7a, 0x27, 0xb4, 0x56,
                                                       0x92, 0x11, 0x90, 0x2e, 0x54, 0x64, 0x31, 0x4c })));
    EXPECT_TRUE(infra::ContentsEqual(privateKeyBin, infra::MakeRange({ 0xff, 0xd1, 0x3b, 0x3b, 0x7f, 0xb7, 0xf0, 0xd4,
                                                        0x60, 0x29, 0x27, 0x0d, 0xc5, 0x7e, 0xa6, 0x75,
                                                        0x2a, 0xbb, 0x2c, 0x98, 0xe9, 0xe2, 0x4c, 0xfa,
                                                        0xb6, 0x34, 0x0a, 0xcb, 0x45, 0x99, 0xc0, 0x61,
                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 })));
}

TEST_F(MaterialGeneratorTest, convert_key_224)
{
    std::vector<std::string> keyFile{ "const std::array<uint8_t, 16> aesKey = { { 0xf9, 0xe4, 0x39, 0x1f, 0x60, 0x80, 0x98, 0x85,",
        "    0x88, 0x6d, 0x44, 0x88, 0xc5, 0xf4, 0xb9, 0xf5 } };",
        "const std::array<uint8_t, 56> ecDsaPublicKey = { { 0x3e, 0x6b, 0xe0, 0x8c, 0x64, 0x3e, 0xf8, 0x26,",
        "    0x3f, 0x9c, 0xdc, 0x7e, 0x75, 0x5d, 0x17, 0x04,",
        "    0xf2, 0xfd, 0xe0, 0xf4, 0x0f, 0x35, 0xc3, 0x01,",
        "    0x9d, 0x8b, 0x2b, 0xb1, 0xd2, 0x87, 0xdb, 0x5e,",
        "    0x92, 0x9e, 0x87, 0x66, 0x59, 0x44, 0xb5, 0xb7,",
        "    0xa5, 0xab, 0x7f, 0xbf, 0xe7, 0x8c, 0x8f, 0xee,",
        "    0x92, 0x11, 0x90, 0x2e, 0x54, 0x64, 0x31, 0x4c } };",
        "const std::array<uint8_t, 56> ecDsaPrivateKey = { { 0xff, 0xd1, 0x3b, 0x3b, 0x7f, 0xb7, 0xf0, 0xd4,",
        "    0x60, 0x29, 0x27, 0x0d, 0xc5, 0x7e, 0xa6, 0x75,",
        "    0x2a, 0xbb, 0x2c, 0x98, 0xe9, 0xe2, 0x4c, 0xfa,",
        "    0xb6, 0x34, 0x0a, 0xcb, 0x45, 0x99, 0xc0, 0x61,",
        "    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,",
        "    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,",
        "    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };};" };
    fileSystem.WriteFile("key224.cpp", keyFile);

    materialGenerator.ImportKeys("key224.cpp");
    materialGenerator.WriteKeysProto("key224.bin");
    auto upgradeKeysProto = fileSystem.ReadBinaryFile("key224.bin");
    infra::ByteInputStream stream(upgradeKeysProto);
    infra::ProtoParser protoParser(stream);
    upgrade_keys::keys224 upgradeKeys(protoParser);
    infra::ConstByteRange aeskeyBin(upgradeKeys.aesKey.symmetricKey.begin(), upgradeKeys.aesKey.symmetricKey.end());
    infra::ConstByteRange publicKeyBin(upgradeKeys.ecDsaKey.publicKey.begin(), upgradeKeys.ecDsaKey.publicKey.end());
    infra::ConstByteRange privateKeyBin(upgradeKeys.ecDsaKey.privateKey.begin(), upgradeKeys.ecDsaKey.privateKey.end());

    EXPECT_TRUE(infra::ContentsEqual(aeskeyBin, infra::MakeRange({ 0xf9, 0xe4, 0x39, 0x1f, 0x60, 0x80, 0x98, 0x85, 0x88, 0x6d, 0x44, 0x88, 0xc5, 0xf4, 0xb9, 0xf5 })));
    EXPECT_TRUE(infra::ContentsEqual(publicKeyBin, infra::MakeRange({ 0x3e, 0x6b, 0xe0, 0x8c, 0x64, 0x3e, 0xf8, 0x26,
                                                       0x3f, 0x9c, 0xdc, 0x7e, 0x75, 0x5d, 0x17, 0x04,
                                                       0xf2, 0xfd, 0xe0, 0xf4, 0x0f, 0x35, 0xc3, 0x01,
                                                       0x9d, 0x8b, 0x2b, 0xb1, 0xd2, 0x87, 0xdb, 0x5e,
                                                       0x92, 0x9e, 0x87, 0x66, 0x59, 0x44, 0xb5, 0xb7,
                                                       0xa5, 0xab, 0x7f, 0xbf, 0xe7, 0x8c, 0x8f, 0xee,
                                                       0x92, 0x11, 0x90, 0x2e, 0x54, 0x64, 0x31, 0x4c })));
    EXPECT_TRUE(infra::ContentsEqual(privateKeyBin, infra::MakeRange({ 0xff, 0xd1, 0x3b, 0x3b, 0x7f, 0xb7, 0xf0, 0xd4,
                                                        0x60, 0x29, 0x27, 0x0d, 0xc5, 0x7e, 0xa6, 0x75,
                                                        0x2a, 0xbb, 0x2c, 0x98, 0xe9, 0xe2, 0x4c, 0xfa,
                                                        0xb6, 0x34, 0x0a, 0xcb, 0x45, 0x99, 0xc0, 0x61,
                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 })));
}