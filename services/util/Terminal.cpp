#include "services/util/Terminal.hpp"
#include "infra/util/Tokenizer.hpp"

namespace services
{
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
}
