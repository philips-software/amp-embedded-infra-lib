#ifndef SERVICES_ELLIPTIC_CURVE_DIFFIE_HELLMAN_HPP
#define SERVICES_ELLIPTIC_CURVE_DIFFIE_HELLMAN_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "rcm/interfaces/EllipticCurve.hpp"

namespace services
{
    class EllipticCurveDiffieHellman
    {
    public:
        explicit EllipticCurveDiffieHellman(EllipticCurveOperations& ecc);

        void CalculateSharedSecretKey(const EllipticCurveExtendedParameters& ellipticCurve, infra::ConstByteRange privateKey, infra::ConstByteRange peerPublicKey, infra::ByteRange sharedSecretKey, const infra::Function<void(bool)>& onDone);

    private:
        class State
        {
        public:
            State(EllipticCurveDiffieHellman& ecdh, const EllipticCurveExtendedParameters* ellipticCurve);
            virtual ~State() = default;

        protected:
            EllipticCurveDiffieHellman& Ecdh() const;
            const EllipticCurveExtendedParameters* EllipticCurve() const;

        private:
            EllipticCurveDiffieHellman& ecdh;
            const EllipticCurveExtendedParameters* ellipticCurve;
        };

        class Idle
            : public State
        {
        public:
            explicit Idle(EllipticCurveDiffieHellman& ecdh);
        };

        class CheckPublicKeyX
            : public State
        {
        public:
            CheckPublicKeyX(EllipticCurveDiffieHellman& ecdh, const EllipticCurveExtendedParameters& ellipticCurve);
        };

        class CheckPublicKeyY
            : public State
        {
        public:
            CheckPublicKeyY(EllipticCurveDiffieHellman& ecdh, const EllipticCurveExtendedParameters& ellipticCurve);
        };

        class CheckPublicKeyOnCurve
            : public State
        {
        public:
            CheckPublicKeyOnCurve(EllipticCurveDiffieHellman& ecdh, const EllipticCurveExtendedParameters& ellipticCurve);
        };

        class CalculateSharedKey
            : public State
        {
        public:
            CalculateSharedKey(EllipticCurveDiffieHellman& ecdh, const EllipticCurveExtendedParameters& ellipticCurve);
        };

    private:
        EllipticCurveOperations& ecc;
        infra::ConstByteRange privateKey;
        infra::ConstByteRange publicKeyX;
        infra::ConstByteRange publicKeyY;
        infra::ByteRange sharedKey;
        infra::Function<void(bool)> onDone;
        infra::PolymorphicVariant<State, Idle, CheckPublicKeyX, CheckPublicKeyY, CheckPublicKeyOnCurve, CalculateSharedKey> states;
    };
}

#endif
