#include "upgrade/pack_builder_instantiations/UpgradePackBuilderApplication.hpp"
#include "upgrade/pack_builder_instantiations/UpgradePackBuilderFacade.hpp"

namespace main_
{
    UpgradePackBuilderApplication::UpgradePackBuilderApplication(const application::UpgradePackBuilder::HeaderInfo& header, const application::SupportedTargets& supportedTargets)
        : header(header)
        , supportedTargets(supportedTargets)
        , parser(header.productName + " Upgrade Pack Builder")
    {
        for (const auto& target : supportedTargets.CmdTargets())
            AddTarget(target);

        for (const auto& target : supportedTargets.HexTargets())
            AddTarget(target);

        for (const auto& [target, location] : supportedTargets.BinTargets())
            AddTarget(target);

        for (const auto& [target, location] : supportedTargets.ElfTargets())
            AddTarget(target);
    }

    int UpgradePackBuilderApplication::Main(int argc, const char* argv[])
    {
        try
        {
            parser.ParseCLI(argc, argv);
        }
        catch (const args::Help&)
        {
            std::cout << parser;
            return 0;
        }
        catch (const args::ParseError& e)
        {
            std::cerr << e.what() << std::endl;
            std::cerr << parser;
            return 1;
        }
        catch (const args::ValidationError& e)
        {
            std::cerr << e.what() << std::endl;
            std::cerr << parser;
            return 1;
        }

        TargetAndFiles requestedTargets;
        for (auto& target : targets)
            requestedTargets.emplace_back(target.Name(), args::get(target), infra::none);

        try
        {
            UpgradePackBuilderFacade(header).Build(supportedTargets, requestedTargets, args::get(outputFile));
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            return 1;
        }

        return 0;
    }

    args::Options UpgradePackBuilderApplication::OptionsForTarget(const std::string& target)
    {
        bool mandatory = std::any_of(supportedTargets.MandatoryTargets().cbegin(), supportedTargets.MandatoryTargets().cend(),
            [&](const auto& s) { return s == target; });
        return mandatory ? args::Options::Required : args::Options::None;
    }

    void UpgradePackBuilderApplication::AddTarget(const std::string& target)
    {
        targets.emplace_back(targetGroup, target, "File for the '" + target + "' target", args::Matcher{ target }, OptionsForTarget(target));
    }
}
