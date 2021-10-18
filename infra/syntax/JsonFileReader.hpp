#ifndef JSON_FILE_READER_HPP
#define JSON_FILE_READER_HPP

#include "infra/syntax/Json.hpp"
#include "hal/interfaces/FileSystem.hpp"
#include "infra/syntax/JsonObjectNavigator.hpp"

namespace infra
{
    class JsonFileReader
    {
    public:
        JsonFileReader(hal::FileSystem& filesystem, const hal::filesystem::path& filename);

        infra::JsonObject& GetJsonObject();
        infra::JsonObjectNavigator& GetNavigator();

    private:
        std::string ReadFileContents(hal::FileSystem& filesystem, const hal::filesystem::path& filename) const;
        void CheckValidJsonObject(infra::JsonObject& jsonObject);

    private:
        std::string fileContents;
        infra::JsonObject json;
        infra::JsonObjectNavigator jsonObjectNavigator;
    };
}

#endif
