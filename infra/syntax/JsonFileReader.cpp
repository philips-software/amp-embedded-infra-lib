#include "infra/syntax/JsonFileReader.hpp"

namespace infra
{
    JsonFileReader::JsonFileReader(const std::vector<std::string>& fileData)
        : fileContents(ReadFileContents(fileData))
        , json(fileContents.c_str())
        , jsonObjectNavigator(json)
    {
        CheckValidJsonObject(json);
    }

    infra::JsonObject& JsonFileReader::GetJsonObject()
    {
        return json;
    }

    infra::JsonObjectNavigator& JsonFileReader::GetNavigator()
    {
        return jsonObjectNavigator;
    }

    std::string JsonFileReader::ReadFileContents(const std::vector<std::string>& fileData) const
    {
        std::string result;

        if (fileData.empty())
            throw JsonFileReaderException("Missing JSON data.");

        for (const auto& line : fileData)
            result.append(line);

        return result;
    }

    void JsonFileReader::CheckValidJsonObject(infra::JsonObject& jsonObject) const
    {
        for (auto it : jsonObject)
            if (it.value.Is<infra::JsonObject>())
                CheckValidJsonObject(it.value.Get<infra::JsonObject>());

        if (jsonObject.Error())
            throw JsonFileReaderException("Invalid JSON object.");
    }
}
