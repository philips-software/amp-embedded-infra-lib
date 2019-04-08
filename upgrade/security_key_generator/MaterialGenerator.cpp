#include "upgrade/security_key_generator/MaterialGenerator.hpp"
#include "crypto/micro-ecc/uECC.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/pk_internal.h"
#include <fstream>
#include <iomanip>
#include <string>

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

        randomDataGenerator.GenerateRandomData(infra::MakeRange(aesKey));
        randomDataGenerator.GenerateRandomData(infra::MakeRange(xteaKey));
        randomDataGenerator.GenerateRandomData(infra::MakeRange(hmacKey));

        int ret;

        ecDsa224PublicKey.resize(ecDsa224KeyLength / 8 * 2);
        ecDsa224PrivateKey.resize(ecDsa224KeyLength / 8 * 2);
        uECC_set_rng(UccRandom);
        ret = uECC_make_key(ecDsa224PublicKey.data(), ecDsa224PrivateKey.data(), uECC_secp224r1());
        if (ret != 1)
            throw std::exception("uECC_make_key returned an error");

        mbedtls_entropy_context entropy;
        mbedtls_entropy_init(&entropy);

        ret = mbedtls_entropy_add_source(&entropy, RandomEntropy, this, 32, MBEDTLS_ENTROPY_SOURCE_STRONG);
        if (ret != 0)
            throw std::exception("mbedtls_entropy_add_source returned an error");

        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_ctr_drbg_init(&ctr_drbg);

        ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, nullptr, 0);
        if (ret != 0)
            throw std::exception("mbedtls_ctr_drbg_seed returned an error");

        mbedtls_pk_init(&pk);

        ret = mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
        if (ret != 0)
            throw std::exception("mbedtls_pk_setup returned an error");

        ret = mbedtls_rsa_gen_key(mbedtls_pk_rsa(pk), mbedtls_ctr_drbg_random, &ctr_drbg,
            rsaKeyLength, 65537);
        if (ret != 0)
            throw std::exception("mbedtls_rsa_gen_key returned an error");

        mbedtls_rsa_context* rsaContext = static_cast<mbedtls_rsa_context*>(pk.pk_ctx);
        mbedtls_rsa_set_padding(rsaContext, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

        ret = mbedtls_rsa_check_privkey(rsaContext);
        if (ret != 0)
            throw std::exception("Public key is invalid");

        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
    }

    MaterialGenerator::~MaterialGenerator()
    {
        mbedtls_pk_free(&pk);
    }

    void MaterialGenerator::WriteKeys(const std::string& fileName)
    {
        std::ofstream file(fileName.c_str());

        if (!file)
            throw std::exception((std::string("Cannot open/create: ") + fileName).c_str());

        mbedtls_rsa_context* rsaContext = static_cast<mbedtls_rsa_context*>(pk.pk_ctx);

        file << R"(#include "upgrade_keys/Keys.hpp"

)";

        PrintVector(file, "aesKey", aesKey);
        PrintVector(file, "xteaKey", xteaKey);
        PrintVector(file, "hmacKey", hmacKey);
        PrintVector(file, "ecDsa224PublicKey", ecDsa224PublicKey);

        PrintMpi(file, "rsaPrivateKeyN", rsaContext->N);
        PrintMpi(file, "rsaPrivateKeyE", rsaContext->E);

        file << R"(#ifdef CCOLA_HOST_BUILD
// Private keys are only available in the upgrade builder, which is compiled only on the host.
// So when compiling for any embedded platform, these keys are not included

)";

        PrintVector(file, "ecDsa224PrivateKey", ecDsa224PrivateKey);

        PrintMpi(file, "rsaPrivateKeyD", rsaContext->D);
        PrintMpi(file, "rsaPrivateKeyP", rsaContext->P);
        PrintMpi(file, "rsaPrivateKeyQ", rsaContext->Q);
        PrintMpi(file, "rsaPrivateKeyDP", rsaContext->DP);
        PrintMpi(file, "rsaPrivateKeyDQ", rsaContext->DQ);
        PrintMpi(file, "rsaPrivateKeyQP", rsaContext->QP);

        file << R"(#endif
)";
    }

    int MaterialGenerator::RandomEntropy(void* data, unsigned char* output, size_t length, size_t* outputLength)
    {
        std::vector<uint8_t> entropy(length);
        reinterpret_cast<MaterialGenerator*>(data)->randomDataGenerator.GenerateRandomData(entropy);
        std::copy(entropy.begin(), entropy.end(), output);
        *outputLength = length;
        return 0;
    }

    int MaterialGenerator::UccRandom(uint8_t* dest, unsigned size)
    {
        std::vector<uint8_t> entropy(size);
        materialGenerator->randomDataGenerator.GenerateRandomData(infra::MakeRange(entropy));
        std::copy(entropy.begin(), entropy.end(), dest);
        return 1;
    }

    void MaterialGenerator::PrintMpi(std::ostream& output, const char* name, mbedtls_mpi number)
    {
        std::vector<uint32_t> n(number.p, number.p + number.n);
        std::vector<uint8_t> m;
        for (uint32_t word : n)
        {
            m.push_back(static_cast<uint8_t>(word));
            m.push_back(static_cast<uint8_t>(word >> 8));
            m.push_back(static_cast<uint8_t>(word >> 16));
            m.push_back(static_cast<uint8_t>(word >> 24));
        }

        PrintVector(output, name, m);
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
    } //TICS !COV_CPP_STREAM_FORMAT_STATE_01
}
