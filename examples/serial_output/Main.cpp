#include "args.hxx"
#ifdef EMIL_HAL_WINDOWS
#include "hal/windows/UartWindows.hpp"
#else
#include "hal/unix/UartUnix.hpp"
#endif
#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include <fstream>

int main(int argc, const char* argv[], const char* env[])
{
    args::ArgumentParser parser("Log all output of a serial port");
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

#ifdef EMIL_HAL_WINDOWS
        hal::UartWindows uart(port);
#else
        hal::UartUnix uart(port);
#endif

        auto runLogger = [&](std::ostream& stream)
        {
            uart.ReceiveData([&stream](infra::ConstByteRange data)
                {
                    stream << infra::ByteRangeAsStdString(data);
                });
            eventDispatcher.Run();
        };

        if (output == "-")
            runLogger(std::cout);
        else
        {
            std::ofstream fileStream{ output, std::ios::binary };
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
