#ifndef INFRA_CRC_CCITT_CALCULATOR_HPP
#define INFRA_CRC_CCITT_CALCULATOR_HPP

#include "infra/util/ByteRange.hpp"

namespace infra
{
    class CrcCcittCalculator
    {
    public:
        void Update(infra::ConstByteRange range);
        uint16_t Result() const;

    private:
        uint16_t crc = 0xffff;
    };
}

#endif
