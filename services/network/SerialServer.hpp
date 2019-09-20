#ifndef SERVICES_SERIAL_SERVER_HPP
#define SERVICES_SERIAL_SERVER_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/event/QueueForOneReaderOneIrqWriter.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "services/network/SingleConnectionListener.hpp"

namespace services
{
    class SerialServerConnectionObserver
        : public services::ConnectionObserver
    {
    public:
        SerialServerConnectionObserver(const infra::ByteRange sendBuffer, const infra::ByteRange receiveBuffer, hal::SerialCommunication& serialCommunication);

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;

    protected:
        virtual void SerialDataReceived();
        virtual void SocketDataReceived();

    private:
        void EvaluatePendingSerialData();

    private:
        const infra::ByteRange sendBuffer;
        const infra::ByteRange receiveBuffer;
        infra::QueueForOneReaderOneIrqWriter<uint8_t> receiveQueue;
        infra::Optional<infra::ByteRange> pendingData;
        hal::SerialCommunication& serialCommunication;
        infra::SharedPtr<infra::StreamReader> streamReader = nullptr;
    };

    class SerialServer
        : public services::SingleConnectionListener
    {
    public:
        template<size_t BufferSize>
        using WithSymmetricBuffer = infra::WithStorage<infra::WithStorage<SerialServer,
            std::array<uint8_t, BufferSize>>,
            std::array<uint8_t, BufferSize>>;

        template<size_t SendBufferSize, size_t ReceiveBufferSize>
        using WithAsymmetricBuffer = infra::WithStorage<infra::WithStorage<SerialServer,
            std::array<uint8_t, SendBufferSize>>,
            std::array<uint8_t, ReceiveBufferSize>>;

        SerialServer(const infra::ByteRange sendBuffer, const infra::ByteRange receiveBuffer, hal::SerialCommunication& serialCommunication, services::ConnectionFactory& connectionFactory, uint16_t port);

    private:
        const infra::ByteRange sendBuffer;
        const infra::ByteRange receiveBuffer;
        hal::SerialCommunication& serialCommunication;
        infra::Creator<services::ConnectionObserver, SerialServerConnectionObserver, void(services::IPAddress address)> connectionCreator;
    };
}

#endif
