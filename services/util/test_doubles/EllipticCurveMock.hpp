#ifndef SERVICES_TEST_DOUBLES_ELLIPTIC_CURVE_MOCK_HPP
#define SERVICES_TEST_DOUBLES_ELLIPTIC_CURVE_MOCK_HPP

#include "services/util/EllipticCurve.hpp"
#include <gmock/gmock.h>

namespace services
{
    class EllipticCurveOperationsMock
        : public EllipticCurveOperations
    {
    public:
        // implementation of EllipticCurveOperations
        MOCK_METHOD(void, ScalarMultiplication, (const EllipticCurveExtendedParameters& curve, infra::ConstByteRange scalar, infra::ConstByteRange x, infra::ConstByteRange y, const MultiplicationResult& onDone), (const, override));
        MOCK_METHOD(void, CheckPointOnCurve, (const EllipticCurveExtendedParameters& curve, infra::ConstByteRange x, infra::ConstByteRange y, const infra::Function<void(PointOnCurveResult)>& onDone), (const, override));
        MOCK_METHOD(void, Comparison, (infra::ConstByteRange a, infra::ConstByteRange b, const infra::Function<void(ComparisonResult)>& onDone), (const, override));
    };
}

#endif
