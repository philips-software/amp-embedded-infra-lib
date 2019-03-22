#ifndef SECURITY_KEY_GENERATOR_MATERIAL_GENERATOR_HPP
#define SECURITY_KEY_GENERATOR_MATERIAL_GENERATOR_HPP

#include "mbedtls/pk.h"
#include "mbedtls/rsa.h"
#include "upgrade/pack_builder/RandomNumberGenerator.hpp"
#include <array>
#include <cstdint>
#include <string>

namespace application
{
    class MaterialGenerator
    {
    public:
        explicit MaterialGenerator(RandomNumberGenerator& randomNumberGenerator);
        ~MaterialGenerator();

        static const std::size_t rsaKeyLength = 1024;
        static const std::size_t aesKeyLength = 128;
        static const std::size_t hmacKeyLength = 256;
        static const std::size_t xteaKeyLength = 128;
        static const std::size_t ecDsa224KeyLength = 224;

        void WriteKeys(const std::string& fileName);

    private:
        void PrintMpi(std::ostream& output, const char* name, mbedtls_mpi number);
        void PrintVector(std::ostream& output, const char* name, const std::vector<uint8_t>& vector);

        static int RandomEntropy(void* data, unsigned char* output, size_t length, size_t* outputLength);
        static int UccRandom(uint8_t* dest, unsigned size);

    private:
        static RandomNumberGenerator* randomNumberGenerator;
        mbedtls_pk_context pk;

        std::vector<uint8_t> aesKey;
        std::vector<uint8_t> xteaKey;
        std::vector<uint8_t> hmacKey;
        std::vector<uint8_t> ecDsa224PublicKey;
        std::vector<uint8_t> ecDsa224PrivateKey;
    };
}

#endif
