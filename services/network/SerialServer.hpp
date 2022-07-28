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
        SerialServerConnectionObserver(const infra::ByteRange receiveBuffer, hal::SerialCommunication& serialCommunication);

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;

    protected:
        virtual void SerialDataReceived();

    private:
        const infra::ByteRange receiveBuffer;
        infra::QueueForOneReaderOneIrqWriter<uint8_t> receiveQueue;
        bool pendingSend = false;
        hal::SerialCommunication& serialCommunication;
        infra::SharedPtr<infra::StreamReader> streamReader = nullptr;
    };

    class SerialServer
        : public services::SingleConnectionListener
    {
    public:
        template<size_t BufferSize>
            using WithBuffer = infra::WithStorage<SerialServer, std::array<uint8_t, BufferSize>>;

        SerialServer(const infra::ByteRange receiveBuffer, hal::SerialCommunication& serialCommunication, services::ConnectionFactory& connectionFactory, uint16_t port);

    private:
        const infra::ByteRange receiveBuffer;
        hal::SerialCommunication& serialCommunication;
        infra::Creator<services::ConnectionObserver, SerialServerConnectionObserver, void(services::IPAddress address)> connectionCreator;
    };
}

#endif
