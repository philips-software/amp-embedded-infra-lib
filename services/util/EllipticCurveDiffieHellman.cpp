#include "services/util/EllipticCurveDiffieHellman.hpp"

namespace services
{
    EllipticCurveDiffieHellman::EllipticCurveDiffieHellman(EllipticCurveOperations& ecc)
        : ecc(ecc)
        , states(infra::InPlaceType<Idle>(), *this)
    {}

    void EllipticCurveDiffieHellman::CalculateSharedSecretKey(const EllipticCurveExtendedParameters& ellipticCurve, infra::ConstByteRange privateKey, infra::ConstByteRange peerPublicKey, infra::ByteRange sharedSecretKey, const infra::Function<void(bool)>& onDone)
    {
        really_assert(privateKey.size() * 2 == peerPublicKey.size() && privateKey.size() == sharedSecretKey.size());

        this->privateKey = privateKey;
        this->publicKeyX = infra::Head(peerPublicKey, peerPublicKey.size() / 2);
        this->publicKeyY = infra::Tail(peerPublicKey, peerPublicKey.size() / 2);
        this->sharedKey = sharedSecretKey;
        this->onDone = onDone;

        states.Emplace<CheckPublicKeyX>(*this, ellipticCurve);
    }

    EllipticCurveDiffieHellman::State::State(EllipticCurveDiffieHellman& ecdh, const EllipticCurveExtendedParameters* ellipticCurve)
        : ecdh(ecdh)
        , ellipticCurve(ellipticCurve)
    {}

    EllipticCurveDiffieHellman& EllipticCurveDiffieHellman::State::Ecdh() const
    {
        return ecdh;
    }

    const EllipticCurveExtendedParameters* EllipticCurveDiffieHellman::State::EllipticCurve() const
    {
        return ellipticCurve;
    }

    EllipticCurveDiffieHellman::Idle::Idle(EllipticCurveDiffieHellman& ecdh)
        : State(ecdh, nullptr)
    {}

    EllipticCurveDiffieHellman::CheckPublicKeyX::CheckPublicKeyX(EllipticCurveDiffieHellman& ecdh, const EllipticCurveExtendedParameters& ellipticCurve)
        : State(ecdh, &ellipticCurve)
    {
        ecdh.ecc.Comparison(ecdh.publicKeyX, ellipticCurve.p, [this](auto result)
            {
                if (result == EllipticCurveOperations::ComparisonResult::aLessThanB)
                    this->Ecdh().states.Emplace<CheckPublicKeyY>(this->Ecdh(), *this->EllipticCurve());
                else
                    this->Ecdh().onDone(false);
            });
    }

    EllipticCurveDiffieHellman::CheckPublicKeyY::CheckPublicKeyY(EllipticCurveDiffieHellman& ecdh, const EllipticCurveExtendedParameters& ellipticCurve)
        : State(ecdh, &ellipticCurve)
    {
        ecdh.ecc.Comparison(ecdh.publicKeyY, ellipticCurve.p, [this](auto result)
            {
                if (result == EllipticCurveOperations::ComparisonResult::aLessThanB)
                    this->Ecdh().states.Emplace<CheckPublicKeyOnCurve>(this->Ecdh(), *this->EllipticCurve());
                else
                    this->Ecdh().onDone(false);
            });
    }

    EllipticCurveDiffieHellman::CheckPublicKeyOnCurve::CheckPublicKeyOnCurve(EllipticCurveDiffieHellman& ecdh, const EllipticCurveExtendedParameters& ellipticCurve)
        : State(ecdh, &ellipticCurve)
    {
        ecdh.ecc.CheckPointOnCurve(ellipticCurve, ecdh.publicKeyX, ecdh.publicKeyY, [this](auto result)
            {
                if (result == EllipticCurveOperations::PointOnCurveResult::pointOnCurve)
                    this->Ecdh().states.Emplace<CalculateSharedKey>(this->Ecdh(), *this->EllipticCurve());
                else
                    this->Ecdh().onDone(false);
            });
    }

    EllipticCurveDiffieHellman::CalculateSharedKey::CalculateSharedKey(EllipticCurveDiffieHellman& ecdh, const EllipticCurveExtendedParameters& ellipticCurve)
        : State(ecdh, &ellipticCurve)
    {
        ecdh.ecc.ScalarMultiplication(ellipticCurve, ecdh.privateKey, ecdh.publicKeyX, ecdh.publicKeyY, [this](auto x, auto)
            {
                infra::Copy(x, this->Ecdh().sharedKey);
                this->Ecdh().onDone(true);
            });
    }
}
