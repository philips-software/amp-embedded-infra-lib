#include "services/network/SerialServer.hpp"

namespace services
{
    SerialServerConnectionObserver::SerialServerConnectionObserver(const infra::ByteRange receiveBuffer, hal::SerialCommunication& serialCommunication)
        : receiveBuffer(receiveBuffer)
        , receiveQueue(receiveBuffer, [this] { SerialDataReceived(); })
        , serialCommunication(serialCommunication)
    {
        serialCommunication.ReceiveData([this](infra::ConstByteRange buffer) { receiveQueue.AddFromInterrupt(buffer); });
    }

    void SerialServerConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);

        while (!receiveQueue.Empty() && stream.Available() > 0)
        {
            auto data = infra::Head(receiveQueue.ContiguousRange(), stream.Available());
            stream << data;
            receiveQueue.Consume(data.size());
        }

        writer = nullptr;
        pendingSend = false;
    }

    void SerialServerConnectionObserver::DataReceived()
    {
        streamReader = ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*streamReader);

        serialCommunication.SendData(stream.ContiguousRange(), [this]
        {
            ConnectionObserver::Subject().AckReceived();
            streamReader = nullptr;
        });
    }

    void SerialServerConnectionObserver::SerialDataReceived()
    {
        if (!receiveQueue.Empty() && !pendingSend)
        {
            pendingSend = true;
            ConnectionObserver::Subject().RequestSendStream(ConnectionObserver::Subject().MaxSendStreamSize());
        }
    }

    SerialServer::SerialServer(const infra::ByteRange receiveBuffer, hal::SerialCommunication& serialCommunication, services::ConnectionFactory& connectionFactory, uint16_t port)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , receiveBuffer(receiveBuffer)
        , serialCommunication(serialCommunication)
        , connectionCreator([this](infra::Optional<SerialServerConnectionObserver>& value, services::IPAddress address)
          {
              value.Emplace(this->receiveBuffer, this->serialCommunication);
          })
    {}
}
