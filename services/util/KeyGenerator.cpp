#include "services/util/KeyGenerator.hpp"

namespace services
{
    KeyGenerator::KeyGenerator(EllipticCurveOperations& ecc)
        : ecc(ecc)
    {}

    void KeyGenerator::GeneratePublicKeyFromPrivateKey(const EllipticCurveExtendedParameters& ellipticCurve, infra::ConstByteRange privateKey, infra::ByteRange publicKey, const infra::Function<void()>& onDone)
    {
        really_assert(privateKey.size() * 2 == publicKey.size());

        this->publicKey = publicKey;
        this->onDone = onDone;

        ecc.ScalarMultiplication(ellipticCurve, privateKey, infra::ConstByteRange(), infra::ConstByteRange(), [this](auto x, auto y)
            {
                infra::Copy(x, infra::Head(this->publicKey, this->publicKey.size() / 2));
                infra::Copy(y, infra::Tail(this->publicKey, this->publicKey.size() / 2));
                this->onDone();
            });
    }
}
