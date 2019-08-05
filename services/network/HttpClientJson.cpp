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

    HttpClientJson::~HttpClientJson()
    {
        if (destructedIndication != nullptr)
            *destructedIndication = true;
    }

    void HttpClientJson::Cancel(const infra::Function<void()>& onDone)
    {
        HttpClientBasic::Cancel(onDone);
    }

    void HttpClientJson::Connected()
    {
        HttpClientObserver::Subject().Get(Path(), Headers());
    }

    void HttpClientJson::ClosingConnection()
    {
        readerPtr = nullptr;
        jsonParser = infra::none;
        services::HttpClientBasic::ClosingConnection();
    }

    void HttpClientJson::StatusAvailable(services::HttpStatusCode statusCode)
    {
        if (statusCode != services::HttpStatusCode::OK)
            ContentError();
    }

    void HttpClientJson::HeaderAvailable(services::HttpHeader header)
    {
        if (header.Field() == "content-type" && header.Value() != "application/json")
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

        if (!destructed)
        {
            readerPtr = nullptr;
            destructedIndication = nullptr;
        }
    }

    void HttpClientJson::Established()
    {
        jsonParser.Emplace(jsonParserCreator, TopJsonObjectVisitor());
    }
}
