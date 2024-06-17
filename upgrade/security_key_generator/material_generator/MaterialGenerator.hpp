#ifndef SECURITY_KEY_GENERATOR_MATERIAL_GENERATOR_HPP
#define SECURITY_KEY_GENERATOR_MATERIAL_GENERATOR_HPP

#include "hal/interfaces/FileSystem.hpp"
#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace application
{
    class MaterialGenerator
    {
    public:
        explicit MaterialGenerator(hal::SynchronousRandomDataGenerator& randomDataGenerator, hal::FileSystem& fileSystem);

        static const std::size_t aesKeyLength = 128;
        static const std::size_t ecDsa224KeyLength = 224;
        static const std::size_t ecDsa256KeyLength = 256;
        enum class EcDsaKey
        {
            ecDsa224,
            ecDsa256
        };

        void WriteKeys(const std::string& fileName);
        void WriteKeysProto(const std::string& fileName);
        void GenerateKeys(EcDsaKey keyType);
        void ImportKeys(const std::string& fileName);

    private:
        static int UccRandom(uint8_t* dest, unsigned size);
        std::string GetRawKey(const std::vector<std::string>& contents, const std::string& keyName) const;
        std::vector<uint8_t> ExtractKey(std::string rawKey) const;
        void PrintVector(std::vector<std::string>& fileContent, const char* name, const std::vector<uint8_t>& vector) const;

    private:
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        hal::FileSystem& fileSystem;
        std::vector<uint8_t> aesKey;
        std::vector<uint8_t> ecDsaPublicKey;
        std::vector<uint8_t> ecDsaPrivateKey;

        std::optional<EcDsaKey> availableKeyType;
    };
}

#endif
