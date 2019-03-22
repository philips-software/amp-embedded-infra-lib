#include "UpgradePackConfigParser.hpp"
#include <complex.h>

namespace
{
    std::pair<std::string, std::string> StdStringPair(infra::JsonString str1, infra::JsonString str2)
    {
        return std::pair<std::string, std::string>(str1.ToStdString(), str2.ToStdString());
    }
}

namespace application
{
    UpgradePackConfigParser::UpgradePackConfigParser(infra::JsonObject& json)
        : json(json)
    {
        CheckValidJson();
        CheckMandatoryKeys();
    }

    void UpgradePackConfigParser::CheckValidJson()
    {
        for (auto it : json)
        {}

        if (json.Error())
            throw ParseException("ConfigParser error: invalid JSON");
    }

    void UpgradePackConfigParser::CheckMandatoryKeys()
    {
        std::vector<std::string> mandatoryKeys = { "components" };
        std::for_each(mandatoryKeys.begin(), mandatoryKeys.end(), [this](const std::string& key)
        {
            infra::BoundedConstString s = key.data();
            if (!json.HasKey(s))
                throw ParseException("ConfigParser error: required key " + key + " missing");
        });
    }

    std::vector<std::pair<std::string, std::string>> UpgradePackConfigParser::GetComponents()
    {
        infra::Optional<infra::JsonObject> components = json.GetOptionalObject("components");

        if (components == infra::none)
            throw ParseException(std::string("ConfigParser error: components list should be an object"));

        std::vector<std::pair<std::string, std::string>> result;

        for (infra::JsonObjectIterator it = components->begin(); it != components->end() ; ++it)
        {
            if (it->value.Is<infra::JsonString>())
                result.push_back(std::make_pair(it->key.ToStdString(), it->value.Get<infra::JsonString>().ToStdString()));
            else
                throw ParseException("ConfigParser error: invalid value for component: " + it->key.ToStdString());
        }

        return result;
    }

    std::vector<std::pair<std::string, std::string>> UpgradePackConfigParser::GetOptions()
    {
        std::vector<std::pair<std::string, std::string>> result;

        if (!json.HasKey("options"))
            return result;

        infra::Optional<infra::JsonObject> options = json.GetOptionalObject("options");
        if (options == infra::none)
            throw ParseException(std::string("ConfigParser error: options list should be an object"));

        for (infra::JsonObjectIterator it = options->begin(); it != options->end(); ++it)
        {
            if (it->value.Is<infra::JsonString>())
                result.push_back(std::make_pair(it->key.ToStdString(), it->value.Get<infra::JsonString>().ToStdString()));
            else if (it->value.Is<int32_t>())
                result.push_back(std::pair<std::string, std::string>(it->key.ToStdString(), std::to_string(it->value.Get<int32_t>())));
            else
                throw ParseException("ConfigParser error: invalid value for option: " + it->key.ToStdString());
        }

        return result;
    }

    std::string UpgradePackConfigParser::GetOutputFilename()
    {
        if (json.HasKey("output_filename"))
        {
            if (json.GetOptionalString("output_filename") == infra::none)
                throw ParseException(std::string("ConfigParser error: output filename should be a string"));
            else
                return json.GetString("output_filename").ToStdString();
        }

        return "";
    }
}
