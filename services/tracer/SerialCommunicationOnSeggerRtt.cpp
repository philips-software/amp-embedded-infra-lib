
#include "services/tracer/SerialCommunicationOnSeggerRtt.hpp"
#include "Config/SEGGER_RTT_Conf.h"
#include "RTT/SEGGER_RTT.h"

namespace services
{
    SerialCommunicationOnSeggerRtt::SerialCommunicationOnSeggerRtt(unsigned int bufferIndex, infra::Duration readInterval)
        : bufferIndex{ bufferIndex }
        , readInterval{ readInterval }
    {
        if (std::exchange(initialized, true) == false)
            SEGGER_RTT_Init();
    }

    void SerialCommunicationOnSeggerRtt::SendData(infra::ConstByteRange range, infra::Function<void()> actionOnCompletion)
    {
        range.shrink_from_back_to(SEGGER_RTT_GetAvailWriteSpace(bufferIndex));
        SEGGER_RTT_Write(bufferIndex, range.cbegin(), range.size());

        actionOnCompletion();
    }

    void SerialCommunicationOnSeggerRtt::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        std::array<uint8_t, BUFFER_SIZE_DOWN> dummyBufferToClearReadBuffer;
        (void)SEGGER_RTT_Read(0, dummyBufferToClearReadBuffer.data(), dummyBufferToClearReadBuffer.size());

        if (!dataReceived)
        {
            this->dataReceived = nullptr;
            readTimer.Cancel();
        }
        else
        {
            this->dataReceived = dataReceived;

            readTimer.Start(readInterval, [this]
                {
                    std::array<uint8_t, BUFFER_SIZE_DOWN> buffer;
                    const auto bytesRead = SEGGER_RTT_Read(0, buffer.data(), buffer.size());

                    if (bytesRead > 0)
                    {
                        auto range = infra::MakeConstByteRange(buffer);
                        range.shrink_from_back_to(bytesRead);
                        this->dataReceived(range);
                    }
                });
        }
    }
}
