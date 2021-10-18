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

        auto fileData = filesystem.ReadFile(filename);
        if (fileData.empty())
            throw hal::EmptyFileException(filename);

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
