#include "hal/interfaces/QuadSpi.hpp"
#include "infra/util/CompareMembers.hpp"

namespace hal
{
    QuadSpi::Lines::Lines(uint8_t instructionLines, uint8_t addressLines, uint8_t alternateLines, uint8_t dataLines)
        : instructionLines(instructionLines)
        , addressLines(addressLines)
        , alternateLines(alternateLines)
        , dataLines(dataLines)
    {}

    QuadSpi::Lines QuadSpi::Lines::SingleSpeed()
    {
        return Lines(1, 1, 1, 1);
    }

    QuadSpi::Lines QuadSpi::Lines::QuadSpeed()
    {
        return Lines(4, 4, 4, 4);
    }

    QuadSpi::Lines QuadSpi::Lines::MixedSpeed(uint8_t instructionLines, uint8_t addressLines, uint8_t alternateLines, uint8_t dataLines)
    {
        return Lines(instructionLines, addressLines, alternateLines, dataLines);
    }

    QuadSpi::Lines QuadSpi::Lines::MixedSpeed(uint8_t instructionLines, uint8_t addressLines, uint8_t dataLines)
    {
        return Lines(instructionLines, addressLines, dataLines, dataLines);
    }

    bool QuadSpi::Lines::operator==(const Lines& other) const
    {
        return infra::Equals()(instructionLines, other.instructionLines)(addressLines, other.addressLines)(alternateLines, other.alternateLines)(dataLines, other.dataLines);
    }

    bool QuadSpi::Lines::operator!=(const Lines& other) const
    {
        return !(*this == other);
    }

    infra::BoundedVector<uint8_t>::WithMaxSize<4> QuadSpi::AddressToVector(uint32_t address, uint8_t numberOfBytes)
    {
        infra::BoundedVector<uint8_t>::WithMaxSize<4> result;

        for (uint8_t i = 0; i != numberOfBytes; ++i)
        {
            result.push_back(static_cast<uint8_t>(address));
            address >>= 8;
        }

        return result;
    }

    uint32_t QuadSpi::VectorToAddress(infra::BoundedVector<uint8_t>::WithMaxSize<4> vector)
    {
        uint32_t result = 0;

        while (!vector.empty())
        {
            result = (result << 8) + vector.back();
            vector.pop_back();
        }

        return result;
    }

    bool QuadSpi::Header::operator==(const Header& other) const
    {
        return infra::Equals()(instruction, other.instruction)(address, other.address)(alternate, other.alternate)(nofDummyCycles, other.nofDummyCycles);
    }

    bool QuadSpi::Header::operator!=(const Header& other) const
    {
        return !(*this == other);
    }
}
