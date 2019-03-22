#ifndef SYNCHRONOUS_HAL_SYNCHRONOUS_QUAD_SPI_HPP
#define SYNCHRONOUS_HAL_SYNCHRONOUS_QUAD_SPI_HPP

#include "infra/util/ByteRange.hpp"
#include <cstdint>

namespace hal
{
    class SynchronousQuadSpi
    {
    public:
        struct Header
        {
            uint8_t instruction;
            uint32_t address;
            uint8_t nofDummyCycles;

            bool operator==(const Header& other) const;
            bool operator!=(const Header& other) const;
        };

    public:
        SynchronousQuadSpi() = default;
        SynchronousQuadSpi(const SynchronousQuadSpi& other) = delete;
        SynchronousQuadSpi& operator=(const SynchronousQuadSpi& other) = delete;

    protected:
        ~SynchronousQuadSpi() = default;

    public:
        virtual void SendData(const Header& header, infra::ConstByteRange data) = 0;
        virtual void SendDataQuad(const Header& header, infra::ConstByteRange data) = 0;
        virtual void ReceiveData(const Header& header, infra::ByteRange data) = 0;
        virtual void ReceiveDataQuad(const Header& header, infra::ByteRange data) = 0;
    };
}

#endif
