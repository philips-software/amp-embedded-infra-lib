#ifndef SERVICES_KEY_GENERATOR_HPP
#define SERVICES_KEY_GENERATOR_HPP

#include "infra/util/ByteRange.hpp"
#include "services/util/EllipticCurve.hpp"

namespace services
{
    class KeyGenerator
    {
    public:
        explicit KeyGenerator(EllipticCurveOperations& ecc);

        void GeneratePublicKeyFromPrivateKey(const EllipticCurveExtendedParameters& ellipticCurve, infra::ConstByteRange privateKey, infra::ByteRange publicKey, const infra::Function<void()>& onDone);

    private:
        EllipticCurveOperations& ecc;
        infra::ByteRange publicKey;
        infra::Function<void()> onDone;
    };
}

#endif
