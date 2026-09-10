#include "services/util/Terminal.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "infra/util/Tokenizer.hpp"

namespace services
{
    namespace
    {
        bool NamesConflict(infra::BoundedConstString first, infra::BoundedConstString second)
        {
            return !first.empty() && first == second;
        }

        bool CommandsConflict(const TerminalCommands::CommandInfo& newCommand, const TerminalCommands::CommandInfo& existingCommand)
        {
            return NamesConflict(newCommand.longName, existingCommand.longName) || NamesConflict(newCommand.longName, existingCommand.shortName) || NamesConflict(newCommand.shortName, existingCommand.longName) || NamesConflict(newCommand.shortName, existingCommand.shortName);
        }
    }

    bool TerminalCommands::ProcessCommand(infra::BoundedConstString data)
    {
        infra::Tokenizer tokenizer(data, ' ');
        infra::BoundedConstString command = tokenizer.Token(0);
        infra::BoundedConstString params = tokenizer.TokenAndRest(1);

        auto commands = Commands();
        auto it = std::find_if(commands.begin(), commands.end(), [command](const Command& entry)
            {
                return (command == entry.info.longName) || (command == entry.info.shortName);
            });

        if (it != commands.end())
        {
            it->function(params);
            return true;
        }
        else
            return false;
    }

    TerminalWithCommandsDuplicateDetector::TerminalWithCommandsDuplicateDetector(TerminalWithCommands& delegate)
        : delegate(delegate)
    {}

    void TerminalWithCommandsDuplicateDetector::RegisterObserver(infra::Observer<TerminalCommands, TerminalWithCommands>* observer)
    {
        observer->Attach(delegate);

        if (!evaluationScheduled)
        {
            evaluationScheduled = true;
            infra::EventDispatcher::Instance().Schedule([this]()
                {
                    evaluationScheduled = false;
                    EvaluateDuplicateCommands();
                });
        }
    }

    void TerminalWithCommandsDuplicateDetector::EvaluateDuplicateCommands()
    {
        delegate.NotifyObservers([this](TerminalCommands& newObserver)
            {
                for (const auto& newCommand : newObserver.Commands())
                    delegate.NotifyObservers([&newObserver, &newCommand](TerminalCommands& existingObserver)
                        {
                            if (&existingObserver != &newObserver)
                                for (const auto& existingCommand : existingObserver.Commands())
                                    really_assert_with_msg(!CommandsConflict(newCommand.info, existingCommand.info),
                                        "Duplicate terminal command '%.*s' (short '%.*s') conflicts with existing command '%.*s' (short '%.*s')",
                                        static_cast<int>(newCommand.info.longName.size()), newCommand.info.longName.data(),
                                        static_cast<int>(newCommand.info.shortName.size()), newCommand.info.shortName.data(),
                                        static_cast<int>(existingCommand.info.longName.size()), existingCommand.info.longName.data(),
                                        static_cast<int>(existingCommand.info.shortName.size()), existingCommand.info.shortName.data());
                        });
            });
    }
}
