#include "hal/generic/SerialCommunicationConsole.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/BoundedString.hpp"

namespace hal
{
    SerialCommunicationConsole::SerialCommunicationConsole()
    {
        std::thread([this]()
            {
                while (true)
                {
                    std::string data;
                    std::getline(std::cin, data);
                    data += "\n";
                    if (dataReceived)
                        dataReceived(infra::StdStringAsByteRange(data));
                } })
            .detach();
    }

    void SerialCommunicationConsole::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        std::cout << infra::ByteRangeAsStdString(data);
        infra::EventDispatcher::Instance().Schedule(actionOnCompletion);
    }

    void SerialCommunicationConsole::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        this->dataReceived = dataReceived;
    }
}
