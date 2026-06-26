#include "services/util/DoubleBufferedSerialCommunication.hpp"

namespace services
{
    DoubleBufferedSerialCommunication::DoubleBufferedSerialCommunication(infra::BoundedVector<uint8_t>& buffer, BufferedSerialCommunication& delegate)
        : buffer(buffer)
        , delegate(delegate)
    {}

    void DoubleBufferedSerialCommunication::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        really_assert(actionOnCompletion != nullptr);
        really_assert(nextActionOnCompletion == nullptr);

        nextSend = data;
        nextActionOnCompletion = actionOnCompletion;

        TrySend();
    }

    infra::StreamReaderWithRewinding& DoubleBufferedSerialCommunication::Reader()
    {
        return delegate.Reader();
    }

    void DoubleBufferedSerialCommunication::AckReceived()
    {
        delegate.AckReceived();
    }

    void DoubleBufferedSerialCommunication::TrySend()
    {
        if (nextActionOnCompletion != nullptr && nowActionOnCompletion == nullptr)
        {
            nowSending = nextSend;
            nowActionOnCompletion = std::move(nextActionOnCompletion);
        }

        if (nowActionOnCompletion != nullptr && !sending)
        {
            sending = true;
            auto insertingChunk = infra::Head(nowSending, buffer.max_size());
            buffer.assign(insertingChunk.begin(), insertingChunk.end());
            nowSending = infra::DiscardHead(nowSending, insertingChunk.size());

            if (nowSending.empty())
                nowActionOnCompletion();

            delegate.SendData(infra::MakeRange(buffer), [this]()
                {
                    sending = false;
                    buffer.clear();

                    TrySend();
                });
        }
    }
}
