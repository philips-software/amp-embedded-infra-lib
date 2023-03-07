#include "hal/generic/TimerServiceGeneric.hpp"
#include "hal/interfaces/FileSystem.hpp"
#ifdef EMIL_HAL_WINDOWS
#include "hal/windows/UartWindows.hpp"
#else
#include "hal/unix/UartUnix.hpp"
#endif
#include "infra/stream/IoOutputStream.hpp"
#include "infra/util/Tokenizer.hpp"
#include "services/network/SerialServer.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"
#include "services/tracer/Tracer.hpp"

struct SerialBridge
{
    SerialBridge(const std::string& serialPortName, services::ConnectionFactory& connectionFactory, int port)
        : serialPort(serialPortName)
        , serialServer(serialPort, connectionFactory, port)
    {}

#ifdef EMIL_HAL_WINDOWS
    hal::UartWindows serialPort;
#else
    hal::UartUnix serialPort;
#endif
    services::SerialServer::WithBuffer<512> serialServer;
};

void ShowUsage(services::Tracer& tracer, const std::string& program)
{
    tracer.Trace() << "Opens a serial port and a TCP socket and forwards data from one to the other." << infra::endl;
    tracer.Trace() << hal::filesystem::path(program).filename().string() << " [-p COMPORT:NETWORKPORT]" << infra::endl;
    tracer.Trace() << "-p\tOpens specified COMPORT and NETWORKPORT and starts forwarding traffic" << infra::endl
                   << "  \tCan be specified multiple times.";

    std::exit(1);
}

int main(int argc, const char* argv[], const char* env[])
{
    static main_::NetworkAdapter network;
    static hal::TimerServiceGeneric timerService;
    static infra::IoOutputStream ioOutputStream;
    static services::Tracer tracer(ioOutputStream);

    static infra::BoundedVector<SerialBridge>::WithMaxSize<8> serialBridges;

    for (int i = 1; i < argc; ++i)
    {
        std::string argument(argv[i]);

        if (argument == "-p" && (i + 1 < argc))
        {
            infra::Tokenizer tokenizer(argv[++i], ':');

            if (tokenizer.Size() == 2)
            {
                std::string comPort{ tokenizer.Token(0).begin(), tokenizer.Token(0).end() };
                std::string networkPort{ tokenizer.Token(1).begin(), tokenizer.Token(1).end() };

                serialBridges.emplace_back(comPort, network.ConnectionFactory(), std::atoi(networkPort.c_str()));
            }
            else
                ShowUsage(tracer, argv[0]);
        }
        else
            ShowUsage(tracer, argv[0]);
    }

    network.Run();
}
