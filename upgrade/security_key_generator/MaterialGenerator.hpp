#ifndef SECURITY_KEY_GENERATOR_MATERIAL_GENERATOR_HPP
#define SECURITY_KEY_GENERATOR_MATERIAL_GENERATOR_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include <array>
#include <cstdint>
#include <string>

namespace application
{
    class MaterialGenerator
    {
    public:
        explicit MaterialGenerator(hal::SynchronousRandomDataGenerator& randomDataGenerator);

        static const std::size_t aesKeyLength = 128;
        static const std::size_t hmacKeyLength = 256;
        static const std::size_t ecDsa224KeyLength = 224;

        void WriteKeys(const std::string& fileName);
        void WriteKeysProto(const std::string& fileName);
        void GenerateKeys();
        void ImportKeys(const std::string& fileName);

    private:
        static int UccRandom(uint8_t* dest, unsigned size);
        std::string GetRawKey(const std::vector<std::string> contents, const std::string keyName);
        std::vector<uint8_t> ExtractKey(std::string rawKey);
        void PrintVector(std::ostream& output, const char* name, const std::vector<uint8_t>& vector);

    private:
        hal::SynchronousRandomDataGenerator& randomDataGenerator;

        std::vector<uint8_t> aesKey;
        std::vector<uint8_t> ecDsa224PublicKey;
        std::vector<uint8_t> ecDsa224PrivateKey;

        bool keysAvailable = false;
    };
}

#endif
