#include "args.hxx"
#include "generated/Version.h"
#include "hal/generic/FileSystemGeneric.hpp"
#include "hal/generic/TimerServiceGeneric.hpp"
#include "services/cucumber/TracingCucumberWireProtocolServer.hpp"
#include "services/network_instantiations/NetworkAdapter.hpp"
#include "services/tracer/TracerOnIoOutputInfrastructure.hpp"
#include <memory>

#if EMIL_BUILD_WIN
    #include <libloaderapi.h>

    struct PluginDeleter
    {
        typedef HMODULE pointer;

        void operator()(HMODULE module)
        {
            FreeLibrary(module);
        }
    };

    using PluginHandle = std::unique_ptr<HMODULE, PluginDeleter>;

    PluginHandle LoadPlugin(const hal::filesystem::path& path)
    {
        auto handle = LoadLibraryW(path.c_str());
        if (handle == nullptr)
            throw std::runtime_error(std::string("Plug-in at path '") + path.string() + "' could not be loaded.");

        return PluginHandle(handle);
    }
#else
    #include <dlfcn.h>

    using PluginHandle = std::unique_ptr<void, int(*)(void*)>;

    PluginHandle LoadPlugin(const hal::filesystem::path& path)
    {
        auto handle = dlopen(path.c_str(), RTLD_LAZY);
        if (handle == nullptr)
            throw std::runtime_error(std::string("Plug-in at path '") + path.string() + "' could not be loaded.");

        return PluginHandle(handle, &dlclose);
    }
#endif

int main(int argc, const char* argv[], const char* env[])
{
    args::ArgumentParser parser(std::string("Cucumber wire proxy ") + emil::generated::VERSION_FULL);
    args::HelpFlag help(parser, "help", "display this help menu.", { 'h', "help" });
    args::PositionalList<hal::filesystem::path> stepPaths(parser, "steps", "Paths to shared libraries with step implementations");

    static hal::TimerServiceGeneric timerService;
    static main_::TracerOnIoOutputInfrastructure tracing;
    static main_::NetworkAdapter network;

    try
    {
        parser.ParseCLI(argc, argv);

        std::vector<PluginHandle> steps;
        for (const auto& path : stepPaths)
            steps.emplace_back(LoadPlugin(path));

        main_::TracingCucumberInfrastructure<4096> wireServer(network.ConnectionFactory(), 90, tracing.tracer);

        network.Run();
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
