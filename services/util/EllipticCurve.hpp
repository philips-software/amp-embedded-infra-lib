#ifndef SERVICES_ELLIPTIC_CURVE_HPP
#define SERVICES_ELLIPTIC_CURVE_HPP

#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"

namespace services
{
    // Parameters of a curve E: y^2 = x^3 + ax + b over a finite field F_p
    struct EllipticCurveParameters
    {
    public:
        infra::ConstByteRange p;  // Finite field modulus
        infra::ConstByteRange a;  // Curve parameter a
        infra::ConstByteRange b;  // Curve parameter b
        infra::ConstByteRange gX; // x-coordinate of the base point G
        infra::ConstByteRange gY; // y-coordinate of the base point G
        infra::ConstByteRange n;  // Order of the base point G
        uint32_t h;               // Cofactor of the base point G
    };

    struct EllipticCurveExtendedParameters
        : EllipticCurveParameters
    {
    public:
        infra::ConstByteRange absA;         // Curve parameter |a|
        bool isAPositive;                   // True if a > 0
        infra::ConstByteRange montgomeryR2; // Montgomery parameter R^2 mod n
    };

    class EllipticCurveOperations
    {
    public:
        enum class PointOnCurveResult : uint8_t
        {
            pointOnCurve,
            pointNotOnCurve,
        };

        enum class ComparisonResult : uint8_t
        {
            aEqualsB,
            aGreaterThanB,
            aLessThanB
        };

        using MultiplicationResult = infra::Function<void(infra::ConstByteRange x, infra::ConstByteRange y)>;

        virtual void ScalarMultiplication(const EllipticCurveExtendedParameters& curve, infra::ConstByteRange scalar, infra::ConstByteRange x, infra::ConstByteRange y, const MultiplicationResult& onDone) const = 0;
        virtual void CheckPointOnCurve(const EllipticCurveExtendedParameters& curve, infra::ConstByteRange x, infra::ConstByteRange y, const infra::Function<void(PointOnCurveResult)>& onDone) const = 0;
        virtual void Comparison(infra::ConstByteRange a, infra::ConstByteRange b, const infra::Function<void(ComparisonResult)>& onDone) const = 0;
    };
}

#endif
