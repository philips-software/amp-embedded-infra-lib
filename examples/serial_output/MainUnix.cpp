#include "args.hxx"
#include "hal/unix/UartUnix.hpp"
#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include <fstream>

namespace
{
    class SerialCommunicationLogger
    {
    public:
        SerialCommunicationLogger(hal::SerialCommunication& serialPort, std::ostream& outputStream)
        {
            serialPort.ReceiveData([&outputStream](infra::ConstByteRange data) { outputStream << infra::ByteRangeAsStdString(data); });
        }
    };
}

int main(int argc, const char* argv[], const char* env[])
{
    std::string toolname = argv[0];
    toolname = toolname.substr(toolname.find_last_of("\\") + 1);
    args::ArgumentParser parser(toolname + " is used to log serial data to an output stream (file, stdout)");
    parser.Prog(toolname);

    args::Group positionals(parser, "Positional arguments:");
    args::Positional<std::string> portArgument(positionals, "port", "port of device to be logged", args::Options::Required);
    args::Positional<std::string> outputArgument(positionals, "output", "output file to write port data to (or - for stdout)", args::Options::Required);

    args::HelpFlag help(parser, "help", "display this help menu.", { 'h', "help" });

    try
    {
        infra::EventDispatcherWithWeakPtr::WithSize<50> eventDispatcher;

        parser.ParseCLI(argc, argv);

        const auto& port = args::get(portArgument);
        const auto& output = args::get(outputArgument);

        hal::UartUnix uart(port);

        auto runLogger = [&](std::ostream& stream) {
            SerialCommunicationLogger logger(uart, stream);
            eventDispatcher.Run();
        };

        if (output == "-")
            runLogger(std::cout);
        else
        {
            std::ofstream fileStream{output, std::ios::binary};
            runLogger(fileStream);
        }
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 1;
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
