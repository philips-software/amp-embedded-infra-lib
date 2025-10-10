#ifndef HAL_QUAD_SPI_HPP
#define HAL_QUAD_SPI_HPP

#include "infra/util/BoundedVector.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"
#include <cstdint>

namespace hal
{
    class QuadSpi
    {
    public:
        struct Header
        {
            std::optional<uint8_t> instruction;
            infra::BoundedVector<uint8_t>::WithMaxSize<4> address;
            infra::BoundedVector<uint8_t>::WithMaxSize<4> alternate;
            uint8_t nofDummyCycles;

            bool operator==(const Header& other) const;
            bool operator!=(const Header& other) const;
        };

        struct Lines
        {
            uint8_t instructionLines = 1;
            uint8_t addressLines = 1;
            uint8_t alternateLines = 1;
            uint8_t dataLines = 1;

            static Lines SingleSpeed();
            static Lines QuadSpeed();
            static Lines MixedSpeed(uint8_t instructionLines, uint8_t addressLines, uint8_t alternateLines, uint8_t dataLines);
            static Lines MixedSpeed(uint8_t instructionLines, uint8_t addressLines, uint8_t dataLines);

            bool operator==(const Lines& other) const;
            bool operator!=(const Lines& other) const;

        private:
            Lines(uint8_t instructionLines, uint8_t addressLines, uint8_t alternateLines, uint8_t dataLines);
        };

        static infra::BoundedVector<uint8_t>::WithMaxSize<4> AddressToVector(uint32_t address, uint8_t numberOfBytes);
        static uint32_t VectorToAddress(infra::BoundedVector<uint8_t>::WithMaxSize<4> vector);

    public:
        QuadSpi() = default;
        QuadSpi(const QuadSpi& other) = delete;
        QuadSpi& operator=(const QuadSpi& other) = delete;

    protected:
        ~QuadSpi() = default;

    public:
        virtual void SendData(const Header& header, infra::ConstByteRange data, Lines lines, const infra::Function<void()>& actionOnCompletion) = 0;
        virtual void ReceiveData(const Header& header, infra::ByteRange data, Lines lines, const infra::Function<void()>& actionOnCompletion) = 0;
        virtual void PollStatus(const Header& header, uint8_t nofBytes, uint32_t match, uint32_t mask, Lines lines, const infra::Function<void()>& actionOnCompletion) = 0;
    };
}

#endif
