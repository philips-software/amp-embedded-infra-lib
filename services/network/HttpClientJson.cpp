#include "services/network/HttpClientJson.hpp"

namespace services
{
    HttpClientJson::HttpClientJson(infra::BoundedString url, const ConnectionInfo& connectionInfo)
        : services::HttpClientBasic(url, connectionInfo.port, connectionInfo.httpClientConnector)
        , jsonParserCreator(connectionInfo.jsonParserCreator)
    {}

    HttpClientJson::HttpClientJson(infra::BoundedString url, const ConnectionInfo& connectionInfo, infra::Duration timeoutDuration)
        : services::HttpClientBasic(url, connectionInfo.port, connectionInfo.httpClientConnector, timeoutDuration)
        , jsonParserCreator(connectionInfo.jsonParserCreator)
    {}

    HttpClientJson::HttpClientJson(infra::BoundedString url, const ConnectionInfo& connectionInfo, NoAutoConnect)
        : services::HttpClientBasic(url, connectionInfo.port, connectionInfo.httpClientConnector, noAutoConnect)
        , jsonParserCreator(connectionInfo.jsonParserCreator)
    {}

    HttpClientJson::HttpClientJson(infra::BoundedString url, const ConnectionInfo& connectionInfo, infra::Duration timeoutDuration, NoAutoConnect)
        : services::HttpClientBasic(url, connectionInfo.port, connectionInfo.httpClientConnector, timeoutDuration, noAutoConnect)
        , jsonParserCreator(connectionInfo.jsonParserCreator)
    {}

    HttpClientJson::~HttpClientJson()
    {
        if (destructedIndication != nullptr)
            *destructedIndication = true;
    }

    void HttpClientJson::Cancel(const infra::Function<void()>& onDone)
    {
        HttpClientBasic::Cancel(onDone);
    }

    void HttpClientJson::Attached()
    {
        HttpClientBasic::Attached();
        HttpClientObserver::Subject().Get(Path(), Headers());
    }

    void HttpClientJson::Detaching()
    {
        readerPtr = nullptr;
        jsonParser = infra::none;
        services::HttpClientBasic::Detaching();
    }

    void HttpClientJson::StatusAvailable(services::HttpStatusCode statusCode)
    {
        HttpClientBasic::StatusAvailable(statusCode);

        if (statusCode != services::HttpStatusCode::OK)
            ContentError();
    }

    void HttpClientJson::HeaderAvailable(services::HttpHeader header)
    {
        if (infra::CaseInsensitiveCompare(header.Field(), "Content-Type") &&
            ((header.Value() != "application/json;charset=UTF-8") && (header.Value() != "application/json")))
            ContentError();
    }

    void HttpClientJson::BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader)
    {
        bool destructed = false;
        destructedIndication = &destructed;

        readerPtr = std::move(reader);

        infra::DataInputStream::WithErrorPolicy stream(*readerPtr, infra::noFail);

        while (!destructed && jsonParser != infra::none && !stream.Empty())
            (*jsonParser)->Feed(infra::ByteRangeAsString(stream.ContiguousRange()));

        // If this object is already destructed, readerPtr cannot be touched anymore
        if (!destructed)
        {
            readerPtr = nullptr;

            // Destroying the reader can lead to destruction, so check again
            if (!destructed)
                destructedIndication = nullptr;
        }
    }

    void HttpClientJson::Established()
    {
        jsonParser.Emplace(jsonParserCreator, TopJsonObjectVisitor());
    }
}
