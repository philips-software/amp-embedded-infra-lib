#include "UpgradePackConfigParser.hpp"

namespace application
{
    UpgradePackConfigParser::UpgradePackConfigParser(infra::JsonObject& json)
        : json(json)
    {
        CheckValidJsonObject(json);
        CheckMandatoryKeys();
    }

    void UpgradePackConfigParser::CheckValidJsonObject(infra::JsonObject& jsonObject)
    {
        for (auto it : jsonObject)
            if (it.value.Is<infra::JsonObject>())
                CheckValidJsonObject(it.value.Get<infra::JsonObject>());

        if (jsonObject.Error())
            throw ParseException("ConfigParser error: invalid JSON");
    }

    void UpgradePackConfigParser::CheckMandatoryKeys()
    {
        std::vector<std::string> mandatoryKeys = { "components" };
        for (const auto& key : mandatoryKeys)
            if (!json.HasKey(key))
                throw ParseException("ConfigParser error: required key " + key + " missing");
    }

    std::vector<std::pair<std::string, std::string>> UpgradePackConfigParser::GetComponents()
    {
        infra::Optional<infra::JsonObject> components = json.GetOptionalObject("components");

        if (components == infra::none)
            throw ParseException(std::string("ConfigParser error: components list should be an object"));

        std::vector<std::pair<std::string, std::string>> result;

        for (infra::JsonObjectIterator it = components->begin(); it != components->end() ; ++it)
            if (it->value.Is<infra::JsonString>())
                result.push_back(std::make_pair(it->key.ToStdString(), it->value.Get<infra::JsonString>().ToStdString()));
            else
                throw ParseException("ConfigParser error: invalid value for component: " + it->key.ToStdString());

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
            if (it->value.Is<infra::JsonString>())
                result.push_back(std::make_pair(it->key.ToStdString(), it->value.Get<infra::JsonString>().ToStdString()));
            else if (it->value.Is<int32_t>())
                result.push_back(std::pair<std::string, std::string>(it->key.ToStdString(), std::to_string(it->value.Get<int32_t>())));
            else
                throw ParseException("ConfigParser error: invalid value for option: " + it->key.ToStdString());

        return result;
    }

    infra::JsonObject UpgradePackConfigParser::GetUpgradeConfiguration()
    {
        if (!json.HasKey("upgrade_configuration"))
            throw ParseException("ConfigParser error: requested key upgrade_configuration is missing");
        
        if (json.GetOptionalObject("upgrade_configuration") == infra::none)
            throw ParseException("ConfigParser error: upgrade_configuration should be an object");
        else
            return json.GetObject("upgrade_configuration");
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

    std::string UpgradePackConfigParser::GetUpgradeKeys()
    {
        if (json.HasKey("upgrade_keys"))
        {
            if (json.GetOptionalString("upgrade_keys") == infra::none)
                throw ParseException(std::string("ConfigParser error: upgrade_keys should be a string"));
            else
                return json.GetString("upgrade_keys").ToStdString();
        }

        return "";
    }

    std::string UpgradePackConfigParser::GetProductName()
    {
        auto upgradeConfiguration = GetUpgradeConfiguration();
        if (upgradeConfiguration.HasKey("product_name"))
        {
            if (upgradeConfiguration.GetOptionalString("product_name") == infra::none)
                throw ParseException(std::string("ConfigParser error: upgrade_configuration/product_name should be a string"));
            else
                return upgradeConfiguration.GetString("product_name").ToStdString();
        }

        return "";
    }

    std::string UpgradePackConfigParser::GetProductVersion()
    {
        auto upgradeConfiguration = GetUpgradeConfiguration();
        if (upgradeConfiguration.HasKey("product_version"))
        {
            if (upgradeConfiguration.GetOptionalString("product_version") == infra::none)
                throw ParseException(std::string("ConfigParser error: upgrade_configuration/product_version should be a string"));
            else
                return upgradeConfiguration.GetString("product_version").ToStdString();
        }

        return "";
    }

    std::string UpgradePackConfigParser::GetComponentName()
    {
        auto upgradeConfiguration = GetUpgradeConfiguration();
        if (upgradeConfiguration.HasKey("component_name"))
        {
            if (upgradeConfiguration.GetOptionalString("component_name") == infra::none)
                throw ParseException(std::string("ConfigParser error: upgrade_configuration/component_name should be a string"));
            else
                return upgradeConfiguration.GetString("component_name").ToStdString();
        }

        return "";
    }

    uint32_t UpgradePackConfigParser::GetComponentVersion()
    {
        auto upgradeConfiguration = GetUpgradeConfiguration();
        if (upgradeConfiguration.HasKey("component_version"))
        {
            if (upgradeConfiguration.GetOptionalInteger("component_version") == infra::none)
                throw ParseException(std::string("ConfigParser error: upgrade_configuration/component_version should be an integer"));
            else
                return upgradeConfiguration.GetInteger("component_version");
        }

        return -1;
    }
}
