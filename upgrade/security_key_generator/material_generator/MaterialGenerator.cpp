#include "upgrade/security_key_generator/material_generator/MaterialGenerator.hpp"
#include "generated/echo/UpgradeKeys.pb.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/syntax/ProtoFormatter.hpp"
#include "infra/util/MemoryRange.hpp"
#include "uECC.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace application
{
    namespace
    {
        MaterialGenerator* materialGenerator = nullptr;
    }

    MaterialGenerator::MaterialGenerator(hal::SynchronousRandomDataGenerator& randomDataGenerator, hal::FileSystem& fileSystem)
        : randomDataGenerator(randomDataGenerator)
        , fileSystem(fileSystem)
    {
        materialGenerator = this;
    }

    void MaterialGenerator::GenerateKeys(EcDsaKey keyType)
    {
        aesKey.resize(aesKeyLength / 8, 0);
        randomDataGenerator.GenerateRandomData(infra::MakeRange(aesKey));

        uECC_set_rng(UccRandom);
        std::size_t keyLength = 0;
        uECC_Curve curve;

        if (keyType == EcDsaKey::ecDsa224)
        {
            keyLength = ecDsa224KeyLength;
            curve = uECC_secp224r1();
        }
        else if (keyType == EcDsaKey::ecDsa256)
        {
            keyLength = ecDsa256KeyLength;
            curve = uECC_secp256r1();
        }
        else
            throw std::runtime_error("not supported key length");

        ecDsaPublicKey.resize(keyLength / 8 * 2);
        ecDsaPrivateKey.resize(keyLength / 8 * 2);
        if (uECC_make_key(ecDsaPublicKey.data(), ecDsaPrivateKey.data(), curve) != 1)
            throw std::runtime_error("uECC_make_key returned an error");

        availableKeyType = keyType;
    }

    void MaterialGenerator::ImportKeys(const std::string& fileName)
    {
        std::vector<std::string> fileContents = fileSystem.ReadFile(fileName);

        auto rawAesKey = GetRawKey(fileContents, "aesKey");
        aesKey = ExtractKey(rawAesKey);

        auto rawEcDsaPrivateKey = GetRawKey(fileContents, "ecDsaPrivateKey");
        ecDsaPrivateKey = ExtractKey(rawEcDsaPrivateKey);

        auto rawEcDsaPublicKey = GetRawKey(fileContents, "ecDsaPublicKey");
        ecDsaPublicKey = ExtractKey(rawEcDsaPublicKey);

        if (ecDsaPrivateKey.size() == ecDsa224KeyLength / 8 * 2)
            availableKeyType = EcDsaKey::ecDsa224;
        else if (ecDsaPrivateKey.size() == ecDsa256KeyLength / 8 * 2)
            availableKeyType = EcDsaKey::ecDsa256;
        else
            throw std::runtime_error("not supported key length");
    }

    void MaterialGenerator::WriteKeys(const std::string& fileName)
    {
        std::vector<std::string> fileContent;

        fileContent.emplace_back(R"(#include "upgrade_keys/Keys.hpp"
)");

        PrintVector(fileContent, "aesKey", aesKey);
        PrintVector(fileContent, "ecDsaPublicKey", ecDsaPublicKey);

        fileContent.emplace_back(R"(#ifdef EMIL_HOST_BUILD
// Private keys are only available in the upgrade builder, which is compiled only on the host.
// So when compiling for any embedded platform, these keys are not included
)");

        PrintVector(fileContent, "ecDsaPrivateKey", ecDsaPrivateKey);

        fileContent.emplace_back(R"(#endif)");
        fileSystem.WriteFile(fileName, fileContent);
    }

    void MaterialGenerator::WriteKeysProto(const std::string& fileName)
    {
        if (!availableKeyType.has_value())
            throw std::runtime_error("Keys are not generated or imported.");

        std::vector<uint8_t> keysPack;
        infra::StdVectorOutputStream keysPackStream(keysPack);
        infra::ProtoFormatter protoFormatter(keysPackStream);
        if (availableKeyType.value() == EcDsaKey::ecDsa224)
        {
            upgrade_keys::keys224 keysProto;
            keysProto.aesKey.symmetricKey.assign(infra::MakeRange(aesKey));
            keysProto.ecDsaKey.privateKey.assign(infra::MakeRange(ecDsaPrivateKey));
            keysProto.ecDsaKey.publicKey.assign(infra::MakeRange(ecDsaPublicKey));
            keysProto.Serialize(protoFormatter);
        }
        else if (availableKeyType.value() == EcDsaKey::ecDsa256)
        {
            upgrade_keys::keys256 keysProto;
            keysProto.aesKey.symmetricKey.assign(infra::MakeRange(aesKey));
            keysProto.ecDsaKey.privateKey.assign(infra::MakeRange(ecDsaPrivateKey));
            keysProto.ecDsaKey.publicKey.assign(infra::MakeRange(ecDsaPublicKey));
            keysProto.Serialize(protoFormatter);
        }

        fileSystem.WriteBinaryFile(fileName, keysPack);
    }

    int MaterialGenerator::UccRandom(uint8_t* dest, unsigned size)
    {
        std::vector<uint8_t> entropy(size);
        materialGenerator->randomDataGenerator.GenerateRandomData(infra::MakeRange(entropy));
        std::copy(entropy.begin(), entropy.end(), dest);
        return 1;
    }

    std::string MaterialGenerator::GetRawKey(const std::vector<std::string>& contents, const std::string& keyName) const
    {
        auto foundKey = false;
        std::string rawKey;

        for (const auto& line : contents)
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

    std::vector<uint8_t> MaterialGenerator::ExtractKey(std::string rawKey) const
    {
        auto innerOpen = rawKey.find_last_of('{');
        auto innerClose = rawKey.find_first_of('}');
        rawKey = rawKey.substr(innerOpen + 1, innerClose - innerOpen - 1);

        std::vector<uint8_t> extractedValues;

        while (rawKey.find(',') != std::string::npos)
        {
            extractedValues.emplace_back(std::stoi(rawKey, nullptr, 16));
            rawKey = rawKey.substr(rawKey.find(',') + 1);
        }

        extractedValues.emplace_back(std::stoi(rawKey, nullptr, 16));

        return extractedValues;
    }

    void MaterialGenerator::PrintVector(std::vector<std::string>& fileContent, const char* name, const std::vector<uint8_t>& vector) const
    {
        std::stringstream ss;
        ss << "const std::array<uint8_t, " << vector.size() << "> " << name << " = { {";

        for (std::size_t i = 0; i != vector.size(); i++)
        {
            if (i % 8 == 0)
                ss << "\n    ";
            ss << "0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(vector[i]) << std::dec;

            if (i != vector.size() - 1)
                ss << ", ";
        }

        ss << "\n} };\n";

        fileContent.emplace_back(ss.str());
    }
}
