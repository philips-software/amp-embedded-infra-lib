#ifndef PACK_BUILDER_CONFIG_PARSER_HPP
#define PACK_BUILDER_CONFIG_PARSER_HPP

#include "infra/syntax/Json.hpp"

namespace application
{
    class UpgradePackConfigParser
    {
    public:
        class ParseException
            : public std::runtime_error
        {
        public:
            ParseException(const std::string& message)
                : std::runtime_error(message)
            {}
        };
    public:
        UpgradePackConfigParser(infra::JsonObject& json);

        std::vector<std::pair<std::string, std::string>> GetComponents();
        std::vector<std::pair<std::string, std::string>> GetOptions();
        std::string GetOutputFilename();
        std::string GetUpgradeKeys();

    private:
        void CheckValidJson();
        void CheckMandatoryKeys();
     
    private:
        infra::JsonObject& json;
    };
}

#endif  // !PACK_BUILDER_CONFIG_PARSER_HPP
