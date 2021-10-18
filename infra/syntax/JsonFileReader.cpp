#include "infra/syntax/JsonFileReader.hpp"

namespace infra
{
    JsonFileReader::JsonFileReader(hal::FileSystem& filesystem, const hal::filesystem::path& filename)
        : fileContents(ReadFileContents(filesystem, filename))
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

    std::string JsonFileReader::ReadFileContents(hal::FileSystem& filesystem, const hal::filesystem::path& filename) const
    {
        std::string result;

        auto data = filesystem.ReadFile(filename);
        if (data.size() == 0)
            throw hal::EmptyFileException(filename);

        for (auto& line : data)
            result.append(line);

        return result;
    }

    void JsonFileReader::CheckValidJsonObject(infra::JsonObject& jsonObject)
    {
        for (auto it : jsonObject)
            if (it.value.Is<infra::JsonObject>())
                CheckValidJsonObject(it.value.Get<infra::JsonObject>());

        if (jsonObject.Error())
            throw std::runtime_error("JsonFileReader: Invalid JSON object.");
    }
}
