#include "services/network/Http.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/Tokenizer.hpp"

namespace services
{
    const HttpHeaders noHeaders{};
    const Chunked chunked;

    namespace http_responses
    {
        const char* const ok = "200 OK";
        const char* const noContent = "204 No Content";
        const char* const badRequest = "400 Bad Request";
        const char* const notFound = "404 Not Found";
        const char* const conflict = "409 Conflict";
        const char* const unprocessableEntity = "422 Unprocessable Entity";
        const char* const tooManyRequests = "429 Too Many Requests";
        const char* const internalServerError = "500 Internal Server Error";
        const char* const notImplemented = "501 Not Implemented";
    }

    namespace
    {
        const char* const separator = ":";
        // Naming is according to rfc7230
        const infra::BoundedConstString httpVersion = "HTTP/1.1";
        const infra::BoundedConstString sp = " ";
        const infra::BoundedConstString crlf = "\r\n";

        std::size_t SchemeEndPositionFromUrl(infra::BoundedConstString url)
        {
            auto schemeEnd = url.find("//");
            if (schemeEnd == infra::BoundedString::npos)
                schemeEnd = 0;
            else
                schemeEnd += 2;

            return schemeEnd;
        }
    }

    HttpHeader::HttpHeader(infra::BoundedConstString field, infra::BoundedConstString value)
        : field(field)
        , value(value)
    {}

    std::size_t HttpHeader::Size() const
    {
        return field.size() + value.size() + std::char_traits<char>::length(separator);
    }

    infra::BoundedConstString HttpHeader::Field() const
    {
        return field;
    }

    infra::BoundedConstString HttpHeader::Value() const
    {
        return value;
    }

    bool HttpHeader::operator==(const HttpHeader& rhs) const
    {
        return rhs.field == field &&
               rhs.value == value;
    }

    HttpRequestFormatter::HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, const HttpHeaders headers)
        : HttpRequestFormatter(verb, hostname, requestTarget, infra::BoundedConstString(), headers)
    {}

    HttpRequestFormatter::HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers)
        : verb(verb)
        , requestTarget(requestTarget.empty() ? "/" : requestTarget)
        , content(content)
        , hostHeader("Host", hostname)
        , headers(headers)
    {
        if (!content.empty())
            AddContentLength(content.size());
    }

    HttpRequestFormatter::HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, std::size_t contentSize, const HttpHeaders headers)
        : verb(verb)
        , requestTarget(requestTarget.empty() ? "/" : requestTarget)
        , hostHeader("Host", hostname)
        , headers(headers)
    {
        AddContentLength(contentSize);
    }

    HttpRequestFormatter::HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, const HttpHeaders headers, Chunked)
        : verb(verb)
        , requestTarget(requestTarget.empty() ? "/" : requestTarget)
        , hostHeader("Host", hostname)
        , headers(headers)
        , chunked(true)
    {}

    std::size_t HttpRequestFormatter::Size() const
    {
        if (!sentHeader)
            return HttpVerbToString(verb).size() + requestTarget.size() + httpVersion.size() + HeadersSize() + (2 * crlf.size()) + (2 * sp.size()) + content.size();
        else
            return content.size();
    }

    std::size_t HttpRequestFormatter::Write(infra::TextOutputStream stream) const
    {
        if (!sentHeader)
        {
            stream << verb << sp << requestTarget << sp << httpVersion << crlf;

            for (const auto& header : headers)
                stream << header << crlf;

            stream << hostHeader << crlf;

            if (contentLengthHeader)
                stream << *contentLengthHeader << crlf;
            if (chunked)
                stream << HttpHeader{ "Transfer-Encoding", "chunked" } << crlf;

            stream << crlf;
        }

        auto available = std::min(stream.Available(), content.size());
        stream << content.substr(0, available);
        return available;
    }

    void HttpRequestFormatter::Consume(std::size_t amount)
    {
        sentHeader = true;
        content = content.substr(amount);
    }

    void HttpRequestFormatter::AddContentLength(std::size_t size)
    {
        infra::StringOutputStream contentLengthStream(contentLength);
        contentLengthStream << size;
        contentLengthHeader.emplace("Content-Length", contentLength);
    }

    std::size_t HttpRequestFormatter::HeadersSize() const
    {
        std::size_t headerSize = 0;
        for (auto&& header : headers)
            headerSize += (header.Size() + crlf.size());

        if (contentLengthHeader)
            headerSize += contentLengthHeader->Size() + crlf.size();
        if (chunked)
            headerSize += HttpHeader("Transfer-Encoding", "chunked").Size() + crlf.size();

        headerSize += hostHeader.Size() + crlf.size();

        return headerSize;
    }

    HttpHeaderParser::HttpHeaderParser(HttpHeaderParserObserver& observer)
        : observer(observer)
    {}

    HttpHeaderParser ::~HttpHeaderParser()
    {
        if (destroyed != nullptr)
            *destroyed = true;
    }

    void HttpHeaderParser::DataReceived(infra::StreamReaderWithRewinding& reader)
    {
        bool localDestroyed = false;
        destroyed = &localDestroyed;

        bool statusLineError = false;
        if (!statusParsed)
            ParseStatusLine(reader, statusLineError);

        if (localDestroyed)
            return;

        destroyed = nullptr;

        if (!statusLineError)
            ParseHeaders(reader);
    }

    void HttpHeaderParser::ParseStatusLine(infra::StreamReaderWithRewinding& reader, bool& error)
    {
        infra::TextInputStream::WithErrorPolicy stream(reader);
        infra::BoundedString::WithStorage<512> headerBuffer;
        headerBuffer.resize(std::min(headerBuffer.max_size(), stream.Available()));
        stream >> headerBuffer;

        auto crlfPos = headerBuffer.find(crlf);
        if (crlfPos != infra::BoundedString::npos)
        {
            statusParsed = true;
            auto statusLine = headerBuffer.substr(0, crlfPos);
            reader.Rewind(statusLine.size() + crlf.size());

            infra::Tokenizer tokenizer(statusLine, ' ');

            auto versionValid = HttpVersionValid(tokenizer.Token(0));
            auto optionalStatusCode = HttpStatusCodeFromString(tokenizer.Token(1));
            if (versionValid && optionalStatusCode)
            {
                statusCode = *optionalStatusCode;
                observer.StatusAvailable(statusCode, statusLine);
            }
            else
            {
                error = true;
                observer.HeaderParsingDone(true);
            }
        }
    }

    bool HttpHeaderParser::HttpVersionValid(infra::BoundedConstString httpVersion)
    {
        static const std::array<infra::BoundedConstString, 2> validVersions{ "HTTP/1.0", "HTTP/1.1" };
        return std::any_of(validVersions.begin(), validVersions.end(), [&](infra::BoundedConstString validVersion)
            {
                return httpVersion == validVersion;
            });
    }

    void HttpHeaderParser::ParseHeaders(infra::StreamReaderWithRewinding& reader)
    {
        infra::TextInputStream::WithErrorPolicy stream(reader);
        infra::BoundedString::WithStorage<512> headerBuffer;
        while (!stream.Empty())
        {
            auto start = reader.ConstructSaveMarker();

            headerBuffer.resize(std::min(headerBuffer.max_size(), stream.Available()));
            stream >> headerBuffer;

            auto crlfPos = headerBuffer.find(crlf);
            if (crlfPos != infra::BoundedString::npos)
            {
                auto headerLine = headerBuffer.substr(0, crlfPos);
                reader.Rewind(start + headerLine.size() + crlf.size());

                if (headerLine.empty() && headerBuffer.size() > crlfPos)
                {
                    observer.HeaderParsingDone(false);
                    return;
                }

                bool localDestroyed = false;
                destroyed = &localDestroyed;

                auto header = HeaderFromString(headerLine);
                observer.HeaderAvailable(header);

                if (localDestroyed)
                    return;
                destroyed = nullptr;
            }
            else if (headerBuffer.full())
            {
                observer.HeaderParsingDone(true);
                return;
            }
            else
            {
                reader.Rewind(start);
                break;
            }
        }
    }

    HttpHeader HttpHeaderParser::HeaderFromString(infra::BoundedConstString header)
    {
        infra::Tokenizer tokenizer(header, ':');
        return { tokenizer.Token(0), infra::TrimLeft(tokenizer.TokenAndRest(1)) };
    }

    infra::BoundedConstString SchemeFromUrl(infra::BoundedConstString url)
    {
        auto schemeEnd = url.find("://");
        if (schemeEnd == infra::BoundedString::npos)
            return infra::BoundedConstString();
        else
            return url.substr(0, schemeEnd);
    }

    infra::BoundedString SchemeFromUrl(infra::BoundedString url)
    {
        auto schemeEnd = url.find("://");
        if (schemeEnd == infra::BoundedString::npos)
            return infra::BoundedString();
        else
            return url.substr(0, schemeEnd);
    }

    infra::BoundedConstString HostAndPortFromUrl(infra::BoundedConstString url)
    {
        auto schemeEnd = SchemeEndPositionFromUrl(url);
        return url.substr(schemeEnd, url.find_first_of("/?", schemeEnd) - schemeEnd);
    }

    infra::BoundedString HostAndPortFromUrl(infra::BoundedString url)
    {
        auto schemeEnd = SchemeEndPositionFromUrl(url);
        return url.substr(schemeEnd, url.find_first_of("/?", schemeEnd) - schemeEnd);
    }

    infra::BoundedConstString HostFromUrl(infra::BoundedConstString url)
    {
        auto hostAndPort = HostAndPortFromUrl(url);
        auto hostAndUserinfo = hostAndPort.substr(0, hostAndPort.find(':'));

        auto atPosition = hostAndUserinfo.find('@');
        if (atPosition != infra::BoundedConstString::npos)
            return hostAndUserinfo.substr(atPosition + 1);
        else
            return hostAndUserinfo;
    }

    infra::BoundedString HostFromUrl(infra::BoundedString url)
    {
        auto hostAndPort = HostAndPortFromUrl(url);
        auto hostAndUserinfo = hostAndPort.substr(0, hostAndPort.find(':'));

        auto atPosition = hostAndUserinfo.find('@');
        if (atPosition != infra::BoundedConstString::npos)
            return hostAndUserinfo.substr(atPosition + 1);
        else
            return hostAndUserinfo;
    }

    infra::Optional<uint16_t> PortFromUrl(infra::BoundedConstString url)
    {
        auto hostAndPort = HostAndPortFromUrl(url);
        auto colonPosition = hostAndPort.find(':');
        if (colonPosition == infra::BoundedConstString::npos)
            return infra::none;

        auto portString = hostAndPort.substr(colonPosition + 1);
        infra::StringInputStream stream(portString, infra::softFail);
        uint16_t port;
        stream >> port;

        if (stream.Failed())
            return infra::none;

        return std::make_optional(port);
    }

    infra::BoundedConstString PathFromUrl(infra::BoundedConstString url)
    {
        auto separator = url.find_first_of("/?", SchemeEndPositionFromUrl(url));
        if (separator == infra::BoundedString::npos)
            return infra::BoundedConstString();
        else
            return url.substr(separator);
    }

    infra::BoundedConstString HttpVerbToString(HttpVerb verb)
    {
        switch (verb)
        {
            case HttpVerb::get:
                return "GET";
            case HttpVerb::head:
                return "HEAD";
            case HttpVerb::connect:
                return "CONNECT";
            case HttpVerb::options:
                return "OPTIONS";
            case HttpVerb::patch:
                return "PATCH";
            case HttpVerb::put:
                return "PUT";
            case HttpVerb::post:
                return "POST";
            case HttpVerb::delete_:
                return "DELETE";
        }

        std::abort();
    }

    infra::Optional<HttpVerb> HttpVerbFromString(infra::BoundedConstString verb)
    {
        if (verb == "GET")
            return std::make_optional(HttpVerb::get);
        else if (verb == "HEAD")
            return std::make_optional(HttpVerb::head);
        else if (verb == "CONNECT")
            return std::make_optional(HttpVerb::connect);
        else if (verb == "OPTIONS")
            return std::make_optional(HttpVerb::options);
        else if (verb == "PATCH")
            return std::make_optional(HttpVerb::patch);
        else if (verb == "PUT")
            return std::make_optional(HttpVerb::put);
        else if (verb == "POST")
            return std::make_optional(HttpVerb::post);
        else if (verb == "DELETE")
            return std::make_optional(HttpVerb::delete_);

        return infra::none;
    }

    infra::Optional<HttpStatusCode> HttpStatusCodeFromString(infra::BoundedConstString statusCode)
    {
        std::underlying_type<services::HttpStatusCode>::type value = 0;

        for (std::size_t index = 0; index < statusCode.size(); ++index)
            value = value * 10 + statusCode[index] - '0';

        switch (value)
        {
            case 100:
                return std::make_optional(HttpStatusCode::Continue);
            case 101:
                return std::make_optional(HttpStatusCode::SwitchingProtocols);
            case 200:
                return std::make_optional(HttpStatusCode::OK);
            case 201:
                return std::make_optional(HttpStatusCode::Created);
            case 202:
                return std::make_optional(HttpStatusCode::Accepted);
            case 203:
                return std::make_optional(HttpStatusCode::NonAuthorativeInformation);
            case 204:
                return std::make_optional(HttpStatusCode::NoContent);
            case 205:
                return std::make_optional(HttpStatusCode::ResetContent);
            case 206:
                return std::make_optional(HttpStatusCode::PartialContent);
            case 300:
                return std::make_optional(HttpStatusCode::MultipleChoices);
            case 301:
                return std::make_optional(HttpStatusCode::MovedPermanently);
            case 302:
                return std::make_optional(HttpStatusCode::Found);
            case 303:
                return std::make_optional(HttpStatusCode::SeeOther);
            case 304:
                return std::make_optional(HttpStatusCode::NotModified);
            case 305:
                return std::make_optional(HttpStatusCode::UseProxy);
            case 307:
                return std::make_optional(HttpStatusCode::TemporaryRedirect);
            case 308:
                return std::make_optional(HttpStatusCode::PermanentRedirect);
            case 400:
                return std::make_optional(HttpStatusCode::BadRequest);
            case 401:
                return std::make_optional(HttpStatusCode::Unauthorized);
            case 402:
                return std::make_optional(HttpStatusCode::PaymentRequired);
            case 403:
                return std::make_optional(HttpStatusCode::Forbidden);
            case 404:
                return std::make_optional(HttpStatusCode::NotFound);
            case 405:
                return std::make_optional(HttpStatusCode::MethodNotAllowed);
            case 406:
                return std::make_optional(HttpStatusCode::NotAcceptable);
            case 407:
                return std::make_optional(HttpStatusCode::ProxyAuthenticationRequired);
            case 408:
                return std::make_optional(HttpStatusCode::RequestTimeOut);
            case 409:
                return std::make_optional(HttpStatusCode::Conflict);
            case 410:
                return std::make_optional(HttpStatusCode::Gone);
            case 411:
                return std::make_optional(HttpStatusCode::LengthRequired);
            case 412:
                return std::make_optional(HttpStatusCode::PreconditionFailed);
            case 413:
                return std::make_optional(HttpStatusCode::RequestEntityTooLarge);
            case 414:
                return std::make_optional(HttpStatusCode::RequestUriTooLarge);
            case 415:
                return std::make_optional(HttpStatusCode::UnsupportedMediaType);
            case 416:
                return std::make_optional(HttpStatusCode::RequestRangeNotSatisfiable);
            case 417:
                return std::make_optional(HttpStatusCode::ExpectationFailed);
            case 500:
                return std::make_optional(HttpStatusCode::InternalServerError);
            case 501:
                return std::make_optional(HttpStatusCode::NotImplemented);
            case 502:
                return std::make_optional(HttpStatusCode::BadGateway);
            case 503:
                return std::make_optional(HttpStatusCode::ServiceUnavailable);
            case 504:
                return std::make_optional(HttpStatusCode::GatewayTimeOut);
            case 505:
                return std::make_optional(HttpStatusCode::HttpVersionNotSupported);
        }

        return infra::none;
    }

    infra::BoundedConstString HttpStatusCodeToString(services::HttpStatusCode statusCode)
    {
        switch (statusCode)
        {
            case services::HttpStatusCode::Continue:
                return "Continue";
            case services::HttpStatusCode::SwitchingProtocols:
                return "SwitchingProtocols";
            case services::HttpStatusCode::OK:
                return "OK";
            case services::HttpStatusCode::Created:
                return "Created";
            case services::HttpStatusCode::Accepted:
                return "Accepted";
            case services::HttpStatusCode::NonAuthorativeInformation:
                return "NonAuthorativeInformation";
            case services::HttpStatusCode::NoContent:
                return "NoContent";
            case services::HttpStatusCode::ResetContent:
                return "ResetContent";
            case services::HttpStatusCode::PartialContent:
                return "PartialContent";
            case services::HttpStatusCode::MultipleChoices:
                return "MultipleChoices";
            case services::HttpStatusCode::MovedPermanently:
                return "MovedPermanently";
            case services::HttpStatusCode::Found:
                return "Found";
            case services::HttpStatusCode::SeeOther:
                return "SeeOther";
            case services::HttpStatusCode::NotModified:
                return "NotModified";
            case services::HttpStatusCode::UseProxy:
                return "UseProxy";
            case services::HttpStatusCode::TemporaryRedirect:
                return "TemporaryRedirect";
            case services::HttpStatusCode::PermanentRedirect:
                return "PermanentRedirect";
            case services::HttpStatusCode::BadRequest:
                return "BadRequest";
            case services::HttpStatusCode::Unauthorized:
                return "Unauthorized";
            case services::HttpStatusCode::PaymentRequired:
                return "PaymentRequired";
            case services::HttpStatusCode::Forbidden:
                return "Forbidden";
            case services::HttpStatusCode::NotFound:
                return "NotFound";
            case services::HttpStatusCode::MethodNotAllowed:
                return "MethodNotAllowed";
            case services::HttpStatusCode::NotAcceptable:
                return "NotAcceptable";
            case services::HttpStatusCode::ProxyAuthenticationRequired:
                return "ProxyAuthenticationRequired";
            case services::HttpStatusCode::RequestTimeOut:
                return "RequestTimeOut";
            case services::HttpStatusCode::Conflict:
                return "Conflict";
            case services::HttpStatusCode::Gone:
                return "Gone";
            case services::HttpStatusCode::LengthRequired:
                return "LengthRequired";
            case services::HttpStatusCode::PreconditionFailed:
                return "PreconditionFailed";
            case services::HttpStatusCode::RequestEntityTooLarge:
                return "RequestEntityTooLarge";
            case services::HttpStatusCode::RequestUriTooLarge:
                return "RequestUriTooLarge";
            case services::HttpStatusCode::UnsupportedMediaType:
                return "UnsupportedMediaType";
            case services::HttpStatusCode::RequestRangeNotSatisfiable:
                return "RequestRangeNotSatisfiable";
            case services::HttpStatusCode::ExpectationFailed:
                return "ExpectationFailed";
            case services::HttpStatusCode::InternalServerError:
                return "InternalServerError";
            case services::HttpStatusCode::NotImplemented:
                return "NotImplemented";
            case services::HttpStatusCode::BadGateway:
                return "BadGateway";
            case services::HttpStatusCode::ServiceUnavailable:
                return "ServiceUnavailable";
            case services::HttpStatusCode::GatewayTimeOut:
                return "GatewayTimeOut";
            case services::HttpStatusCode::HttpVersionNotSupported:
                return "HttpVersionNotSupported";
        }

        std::abort();
    }
}

namespace infra
{
    TextOutputStream& operator<<(TextOutputStream& stream, const services::HttpHeader& header)
    {
        stream << header.Field() << services::separator << header.Value();

        return stream;
    }

    TextOutputStream& operator<<(TextOutputStream& stream, services::HttpVerb verb)
    {
        stream << services::HttpVerbToString(verb);

        return stream;
    }

    TextOutputStream& operator<<(TextOutputStream& stream, services::HttpStatusCode statusCode)
    {
        stream << services::HttpStatusCodeToString(statusCode);

        return stream;
    }
}
