#ifndef UPGRADE_PACK_BUILD_LIBRARY_IMAGE_SIGNER_RSA_HPP
#define UPGRADE_PACK_BUILD_LIBRARY_IMAGE_SIGNER_RSA_HPP

#include "upgrade/pack/KeyDefinitions.hpp"
#include "upgrade/pack_builder/ImageSigner.hpp"
#include "upgrade/pack_builder/RandomNumberGenerator.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace application
{
    class ImageSignerRsa
        : public ImageSigner
    {
    public:
        ImageSignerRsa(RandomNumberGenerator& randomNumberGenerator, const RsaPublicKey& publicKey, const RsaPrivateKey& privateKey);

        virtual uint16_t SignatureMethod() const override;
        virtual uint16_t SignatureLength() const override;
        virtual std::vector<uint8_t> ImageSignature(const std::vector<uint8_t>& image) override;
        virtual bool CheckSignature(const std::vector<uint8_t>& signature, const std::vector<uint8_t>& image) override;

        static const uint16_t signatureMethod = 1;
        static const size_t keyLength = 1024;

    private:
        void CalculateSha256(const std::vector<uint8_t>& image);
        void CalculateSignature(const std::vector<uint8_t>& image);

        static int MyRandom(void *rng_state, unsigned char *output, size_t len);

    private:
        const RsaPublicKey& publicKey;
        const RsaPrivateKey& privateKey;
        static RandomNumberGenerator* randomNumberGenerator;
        std::string keyFileName;
        std::array<uint8_t, 32> hash;
        std::array<uint8_t, keyLength / 8> signature;
    };
}

#endif
