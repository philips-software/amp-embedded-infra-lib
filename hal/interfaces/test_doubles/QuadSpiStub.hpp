#ifndef HAL_QUAD_SPI_STUB_HPP
#define HAL_QUAD_SPI_STUB_HPP

#include "gmock/gmock.h"
#include "hal/interfaces/QuadSpi.hpp"
#include "infra/util/AutoResetFunction.hpp"

namespace hal
{
    class QuadSpiStub
        : public QuadSpi
    {
    public:
        virtual void SendData(const Header& header, infra::ConstByteRange data, Lines lines, const infra::Function<void()>& actionOnCompletion) override;
        virtual void ReceiveData(const Header& header, infra::ByteRange data, Lines lines, const infra::Function<void()>& actionOnCompletion) override;
        virtual void PollStatus(const Header& header, uint8_t nofBytes, uint32_t match, uint32_t mask, Lines lines, const infra::Function<void()>& actionOnCompletion) override;

        MOCK_METHOD3(SendDataMock, void(const Header&, infra::ConstByteRange, Lines lines));
        MOCK_METHOD2(ReceiveDataMock, infra::ConstByteRange(const Header&, Lines lines));
        MOCK_METHOD5(PollStatusMock, void(const Header&, uint8_t nofBytes, uint32_t match, uint32_t mask, Lines lines));

        infra::AutoResetFunction<void()> onDone;
    };
}

#endif
