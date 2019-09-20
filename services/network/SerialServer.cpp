#include "services/network/SerialServer.hpp"

namespace services
{
    SerialServerConnectionObserver::SerialServerConnectionObserver(infra::ByteRange sendBuffer, infra::ByteRange receiveBuffer, hal::SerialCommunication& serialCommunication)
        : sendBuffer(sendBuffer)
        , receiveBuffer(receiveBuffer)
        , receiveQueue(receiveBuffer, [this] { SerialDataReceived(); })
        , serialCommunication(serialCommunication)
    {
        serialCommunication.ReceiveData([this](infra::ConstByteRange buffer) { receiveQueue.AddFromInterrupt(buffer); });
    }

    void SerialServerConnectionObserver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);

        stream << *pendingData;
        receiveQueue.Consume(pendingData->size());
        pendingData = infra::none;
        writer = nullptr;

        if (!receiveQueue.Empty())
            EvaluatePendingSerialData();
    }

    void SerialServerConnectionObserver::DataReceived()
    {
        SocketDataReceived();
    }

    void SerialServerConnectionObserver::SerialDataReceived()
    {
        EvaluatePendingSerialData();
    }

    void SerialServerConnectionObserver::SocketDataReceived()
    {
        streamReader = ConnectionObserver::Subject().ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*streamReader);

        serialCommunication.SendData(stream.ContiguousRange(), [this]
        {
            ConnectionObserver::Subject().AckReceived();
            streamReader = nullptr;
        });
    }

    void SerialServerConnectionObserver::EvaluatePendingSerialData()
    {
        if (!pendingData)
        {
            auto receivedData = receiveQueue.ContiguousRange();
            pendingData.Emplace(infra::Head(sendBuffer, receivedData.size()));
            infra::Copy(receivedData, *pendingData);

            ConnectionObserver::Subject().RequestSendStream(pendingData->size());
        }
    }

    SerialServer::SerialServer(infra::ByteRange sendBuffer, infra::ByteRange receiveBuffer, hal::SerialCommunication& serialCommunication, services::ConnectionFactory& connectionFactory, uint16_t port)
        : SingleConnectionListener(connectionFactory, port, { connectionCreator })
        , sendBuffer(sendBuffer)
        , receiveBuffer(receiveBuffer)
        , serialCommunication(serialCommunication)
        , connectionCreator([this](infra::Optional<SerialServerConnectionObserver>& value, services::IPAddress address)
          {
              value.Emplace(this->sendBuffer, this->receiveBuffer, this->serialCommunication);
          })
    {}
}
