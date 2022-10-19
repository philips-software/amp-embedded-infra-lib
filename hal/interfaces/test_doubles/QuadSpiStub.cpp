#include "hal/interfaces/test_doubles/QuadSpiStub.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace hal
{
    void QuadSpiStub::SendData(const Header& header, infra::ConstByteRange data, Lines lines, const infra::Function<void()>& actionOnCompletion)
    {
        assert(!onDone);
        onDone = actionOnCompletion;
        SendDataMock(header, data, lines);
        infra::EventDispatcher::Instance().Schedule([this]()
            { onDone(); });
    }

    void QuadSpiStub::ReceiveData(const Header& header, infra::ByteRange data, Lines lines, const infra::Function<void()>& actionOnCompletion)
    {
        assert(!onDone);
        onDone = actionOnCompletion;
        infra::ConstByteRange result = ReceiveDataMock(header, lines);
        std::copy(result.begin(), result.end(), data.begin());
        infra::EventDispatcher::Instance().Schedule([this]()
            { onDone(); });
    }

    void QuadSpiStub::PollStatus(const Header& header, uint8_t nofBytes, uint32_t match, uint32_t mask, Lines lines, const infra::Function<void()>& actionOnCompletion)
    {
        assert(!onDone);
        onDone = actionOnCompletion;
        PollStatusMock(header, nofBytes, match, mask, lines);
    }
}
