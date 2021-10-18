#ifndef JSON_FILE_READER_HPP
#define JSON_FILE_READER_HPP

#include "infra/syntax/Json.hpp"
#include "hal/interfaces/FileSystem.hpp"
#include "infra/syntax/JsonObjectNavigator.hpp"

namespace infra
{
    class JsonFileReader
    {
    protected:
        class JsonFileReaderException
            : public std::runtime_error
        {
        public:
            explicit JsonFileReaderException(const std::string& message)
                : std::runtime_error(std::string("JsonFileReader: ") + message)
            {}
        };

    public:
        JsonFileReader(hal::FileSystem& filesystem, const hal::filesystem::path& filename);

        infra::JsonObject& GetJsonObject();
        infra::JsonObjectNavigator& GetNavigator();

    private:
        std::string ReadFileContents(hal::FileSystem& filesystem, const hal::filesystem::path& filename) const;
        void CheckValidJsonObject(infra::JsonObject& jsonObject) const;

    private:
        std::string fileContents;
        infra::JsonObject json;
        infra::JsonObjectNavigator jsonObjectNavigator;
    };
}

#endif
