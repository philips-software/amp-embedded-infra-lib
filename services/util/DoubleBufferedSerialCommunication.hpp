#ifndef SERVICES_DOUBLE_BUFFERED_SERIAL_COMMUNICATION_HPP
#define SERVICES_DOUBLE_BUFFERED_SERIAL_COMMUNICATION_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/BoundedVector.hpp"

namespace services
{
    class DoubleBufferedSerialCommunication
        : public hal::BufferedSerialCommunication
        , private hal::BufferedSerialCommunicationObserver
    {
    public:
        template<std::size_t Size>
        using WithStorage = infra::WithStorage<DoubleBufferedSerialCommunication, infra::BoundedVector<uint8_t>::WithMaxSize<Size>>;

        DoubleBufferedSerialCommunication(infra::BoundedVector<uint8_t>& buffer, hal::BufferedSerialCommunication& delegate);

        // Implementation of BufferedSerialCommunication
        void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
        infra::StreamReaderWithRewinding& Reader() override;
        void AckReceived() override;

    private:
        void DataReceived() override;

    private:
        void TrySend();

    private:
        infra::BoundedVector<uint8_t>& buffer;
        hal::BufferedSerialCommunication& delegate;
        infra::ConstByteRange nextSend;
        infra::AutoResetFunction<void()> nextActionOnCompletion;

        infra::ConstByteRange nowSending;
        infra::AutoResetFunction<void()> nowActionOnCompletion;

        bool sending = false;
    };
}

#endif
