#include "upgrade/security_key_generator/MaterialGenerator.hpp"
#include "crypto/micro-ecc/uECC.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/pk_internal.h"
#include <fstream>
#include <iomanip>
#include <string>
#include "infra/syntax/ProtoFormatter.hpp"
#include "hal/generic/FileSystemGeneric.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "generated/echo/UpgradeKeys.pb.hpp"

namespace application
{
    namespace
    {
        MaterialGenerator* materialGenerator = nullptr;
    }

    MaterialGenerator::MaterialGenerator(hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : randomDataGenerator(randomDataGenerator)
    {
        materialGenerator = this;
    }

    MaterialGenerator::~MaterialGenerator()
    {
    }

    void MaterialGenerator::GenerateKeys()
    {
        aesKey.resize(aesKeyLength / 8, 0);
        randomDataGenerator.GenerateRandomData(infra::MakeRange(aesKey));

        ecDsa224PublicKey.resize(ecDsa224KeyLength / 8 * 2);
        ecDsa224PrivateKey.resize(ecDsa224KeyLength / 8 * 2);
        uECC_set_rng(UccRandom);
        auto ret = uECC_make_key(ecDsa224PublicKey.data(), ecDsa224PrivateKey.data(), uECC_secp224r1());
        if (ret != 1)
            throw std::runtime_error("uECC_make_key returned an error");

        keysAvailable = true;
    }

    void MaterialGenerator::ImportKeys(const std::string& fileName)
    {
        hal::FileSystemGeneric fileSystem;
        std::vector<std::string> fileContents = fileSystem.ReadFile(fileName);

        auto rawAesKey = GetRawKey(fileContents, "aesKey");
        aesKey = ExtractKey(rawAesKey);

        auto rawEcDsaPrivateKey = GetRawKey(fileContents, "ecDsa224PrivateKey");
        ecDsa224PrivateKey = ExtractKey(rawEcDsaPrivateKey);

        auto rawEcDsaPublicKey = GetRawKey(fileContents, "ecDsa224PublicKey");
        ecDsa224PublicKey = ExtractKey(rawEcDsaPublicKey);

        keysAvailable = true;
    }

    void MaterialGenerator::WriteKeys(const std::string& fileName)
    {
        std::ofstream file(fileName.c_str());

        if (!file)
            throw std::runtime_error((std::string("Cannot open/create: ") + fileName).c_str());


        file << R"(#include "upgrade_keys/Keys.hpp"

)";

        PrintVector(file, "aesKey", aesKey);
        PrintVector(file, "ecDsa224PublicKey", ecDsa224PublicKey);

        file << R"(#ifdef CCOLA_HOST_BUILD
// Private keys are only available in the upgrade builder, which is compiled only on the host.
// So when compiling for any embedded platform, these keys are not included

)";

        PrintVector(file, "ecDsa224PrivateKey", ecDsa224PrivateKey);

        file << R"(#endif
)";
    }

    void MaterialGenerator::WriteKeysProto(const std::string& fileName)
    {
        if (!keysAvailable)
            throw std::runtime_error("Keys are not generated or imported.");

        upgrade_keys::keys keysProto;
        
        infra::MemoryRange<uint8_t> aesKeyRange(aesKey);
        keysProto.aesKey.symmetricKey.assign(aesKeyRange);

        infra::MemoryRange<uint8_t> ecDsa224PrivateKeyRange(ecDsa224PrivateKey);
        keysProto.ecDsaKey.privateKey.assign(ecDsa224PrivateKeyRange);

        infra::MemoryRange<uint8_t> ecDsa224PublicKeyRange(ecDsa224PublicKey);
        keysProto.ecDsaKey.publicKey.assign(ecDsa224PublicKeyRange);

        std::vector<uint8_t> keysPack;
        infra::StdVectorOutputStream keysPackStream(keysPack);
        infra::ProtoFormatter protoFormatter(keysPackStream);
        keysProto.Serialize(protoFormatter);

        hal::FileSystemGeneric fileSystem;
        fileSystem.WriteBinaryFile(fileName, keysPack);
    }

    int MaterialGenerator::UccRandom(uint8_t* dest, unsigned size)
    {
        std::vector<uint8_t> entropy(size);
        materialGenerator->randomDataGenerator.GenerateRandomData(infra::MakeRange(entropy));
        std::copy(entropy.begin(), entropy.end(), dest);
        return 1;
    }

    std::string MaterialGenerator::GetRawKey(const std::vector<std::string> contents, const std::string keyName)
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

    std::vector<uint8_t> MaterialGenerator::ExtractKey(std::string rawKey)
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

    void MaterialGenerator::PrintVector(std::ostream& output, const char* name, const std::vector<uint8_t>& vector)
    {
        std::ios oldState(nullptr);
        oldState.copyfmt(output);

        output << "const std::array<uint8_t, " << vector.size() << "> " << name << " = { {";

        for (std::size_t i = 0; i != vector.size(); i++)
        {
            if (i % 8 == 0)
                output << "\n    ";
            output << "0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(vector[i]) << std::dec;

            if (i != vector.size() - 1)
                output << ", ";
        }

        output << "\n} };\n\n";

        output.copyfmt(oldState);
    }
}
