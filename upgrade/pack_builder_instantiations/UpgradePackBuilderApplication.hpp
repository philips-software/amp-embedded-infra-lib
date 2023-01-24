#include "args.hxx"
#include "upgrade/pack_builder/SupportedTargets.hpp"
#include "upgrade/pack_builder/UpgradePackBuilder.hpp"
#include <list>
#include <string>

namespace main_
{
    class UpgradePackBuilderApplication
    {
    public:
        UpgradePackBuilderApplication(const application::UpgradePackBuilder::HeaderInfo& header, const application::SupportedTargets& targets);

        int Main(int argc, const char* argv[]);

    private:
        args::Options OptionsForTarget(const std::string& target);
        void AddTarget(const std::string& target);

    private:
        const application::UpgradePackBuilder::HeaderInfo& header;
        const application::SupportedTargets& supportedTargets;

        args::ArgumentParser parser;
        args::HelpFlag help{ parser, "help", "Display this help menu", {'h', "help"} };
        args::ValueFlag<std::string> outputFile{ parser, "filename", "Output file name", {'o', "output"}, args::Options::Required };
        args::Group targetGroup{ parser, "Supported Targets" };
        std::list<args::ValueFlag<std::string>> targets;
    };
}
