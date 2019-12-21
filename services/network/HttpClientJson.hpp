#ifndef SERVICES_NETWORK_HTTP_CLIENT_JSON
#define SERVICES_NETWORK_HTTP_CLIENT_JSON

#include "infra/syntax/JsonStreamingParser.hpp"
#include "infra/util/ProxyCreator.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/network/HttpClientBasic.hpp"

namespace services
{
    class HttpClientJson
        : protected HttpClientBasic
    {
    public:
        using JsonParserCreatorBase = infra::CreatorBase<infra::JsonStreamingObjectParser, void(infra::JsonObjectVisitor&)>;

        template<std::size_t MaxTagLength, std::size_t MaxValueLength, std::size_t MaxObjectNesting>
            using JsonParserCreator = JsonParserCreatorBase::WithCreator<infra::JsonStreamingObjectParser::WithBuffers<MaxTagLength, MaxValueLength, MaxObjectNesting>>;

        struct ConnectionInfo
        {
            JsonParserCreatorBase& jsonParserCreator;
            uint16_t port;
            services::HttpClientConnector& httpClientConnector;
        };

        HttpClientJson(infra::BoundedString url, const ConnectionInfo& connectionInfo);
        HttpClientJson(infra::BoundedString url, const ConnectionInfo& connectionInfo, infra::Duration timeoutDuration);
        ~HttpClientJson();

        void Cancel(const infra::Function<void()>& onDone);

    protected:
        virtual infra::JsonObjectVisitor& TopJsonObjectVisitor() = 0;

        // Implementation of HttpClientObserver
        virtual void Connected() override;  // Default implementation. Override for behaviour other than an HTTP GET request
        virtual void Detaching() override;
        virtual void StatusAvailable(services::HttpStatusCode statusCode) override;
        virtual void HeaderAvailable(services::HttpHeader header) override;
        virtual void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) override;

        // Override HttpClient methods
        virtual void Established() override;

    private:
        JsonParserCreatorBase& jsonParserCreator;
        infra::Optional<infra::ProxyCreator<JsonParserCreatorBase&>> jsonParser;

        infra::SharedPtr<infra::StreamReader> readerPtr;
        bool* destructedIndication = nullptr;
    };
}

#endif
