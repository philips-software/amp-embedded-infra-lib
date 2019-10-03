#include "services/network/Http.hpp"

namespace services
{
    namespace
    {
        static const char* separator = ":";

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

    infra::BoundedConstString SchemeFromUrl(infra::BoundedConstString url)
    {
        auto schemeEnd = url.find("://");
        if (schemeEnd == infra::BoundedString::npos)
            return infra::BoundedConstString();
        else
            return url.substr(0, schemeEnd);
    }

    infra::BoundedConstString HostFromUrl(infra::BoundedConstString url)
    {
        auto schemeEnd = SchemeEndPositionFromUrl(url);
        return url.substr(schemeEnd, url.find('/', schemeEnd) - schemeEnd);
    }

    infra::BoundedConstString PathFromUrl(infra::BoundedConstString url)
    {
        auto separator = url.find('/', SchemeEndPositionFromUrl(url));
        if (separator == infra::BoundedString::npos)
            return infra::BoundedConstString();
        else
            return url.substr(separator);
    }

    infra::BoundedConstString HttpVerbToString(HttpVerb verb)
    {
        switch (verb)
        {
        case HttpVerb::get: return "GET";
        case HttpVerb::head: return "HEAD";
        case HttpVerb::connect: return "CONNECT";
        case HttpVerb::options: return "OPTIONS";
        case HttpVerb::patch: return "PATCH";
        case HttpVerb::put: return "PUT";
        case HttpVerb::post: return "POST";
        case HttpVerb::delete_: return "DELETE";
        }

        std::abort();
    }

    infra::Optional<HttpVerb> HttpVerbFromString(infra::BoundedConstString verb)
    {
        if (verb == "GET")
            return infra::MakeOptional(HttpVerb::get);
        else if (verb == "HEAD")
            return infra::MakeOptional(HttpVerb::head);
        else if (verb == "CONNECT")
            return infra::MakeOptional(HttpVerb::connect);
        else if (verb == "OPTIONS")
            return infra::MakeOptional(HttpVerb::options);
        else if (verb == "PATCH")
            return infra::MakeOptional(HttpVerb::patch);
        else if (verb == "PUT")
            return infra::MakeOptional(HttpVerb::put);
        else if (verb == "POST")
            return infra::MakeOptional(HttpVerb::post);
        else if (verb == "DELETE")
            return infra::MakeOptional(HttpVerb::delete_);

        return infra::none;
    }

    infra::Optional<HttpStatusCode> HttpStatusCodeFromString(infra::BoundedConstString statusCode)
    {
        std::underlying_type<services::HttpStatusCode>::type value = 0;

        for (std::size_t index = 0; index < statusCode.size(); ++index)
            value = value * 10 + statusCode[index] - '0';

        switch (value)
        {
        case 100: return infra::MakeOptional(HttpStatusCode::Continue);
        case 101: return infra::MakeOptional(HttpStatusCode::SwitchingProtocols);
        case 200: return infra::MakeOptional(HttpStatusCode::OK);
        case 201: return infra::MakeOptional(HttpStatusCode::Created);
        case 202: return infra::MakeOptional(HttpStatusCode::Accepted);
        case 203: return infra::MakeOptional(HttpStatusCode::NonAuthorativeInformation);
        case 204: return infra::MakeOptional(HttpStatusCode::NoContent);
        case 205: return infra::MakeOptional(HttpStatusCode::ResetContent);
        case 206: return infra::MakeOptional(HttpStatusCode::PartialContent);
        case 300: return infra::MakeOptional(HttpStatusCode::MultipleChoices);
        case 301: return infra::MakeOptional(HttpStatusCode::MovedPermanently);
        case 302: return infra::MakeOptional(HttpStatusCode::Found);
        case 303: return infra::MakeOptional(HttpStatusCode::SeeOther);
        case 304: return infra::MakeOptional(HttpStatusCode::NotModified);
        case 305: return infra::MakeOptional(HttpStatusCode::UseProxy);
        case 307: return infra::MakeOptional(HttpStatusCode::TemporaryRedirect);
        case 400: return infra::MakeOptional(HttpStatusCode::BadRequest);
        case 401: return infra::MakeOptional(HttpStatusCode::Unauthorized);
        case 402: return infra::MakeOptional(HttpStatusCode::PaymentRequired);
        case 403: return infra::MakeOptional(HttpStatusCode::Forbidden);
        case 404: return infra::MakeOptional(HttpStatusCode::NotFound);
        case 405: return infra::MakeOptional(HttpStatusCode::MethodNotAllowed);
        case 406: return infra::MakeOptional(HttpStatusCode::NotAcceptable);
        case 407: return infra::MakeOptional(HttpStatusCode::ProxyAuthenticationRequired);
        case 408: return infra::MakeOptional(HttpStatusCode::RequestTimeOut);
        case 409: return infra::MakeOptional(HttpStatusCode::Conflict);
        case 410: return infra::MakeOptional(HttpStatusCode::Gone);
        case 411: return infra::MakeOptional(HttpStatusCode::LengthRequired);
        case 412: return infra::MakeOptional(HttpStatusCode::PreconditionFailed);
        case 413: return infra::MakeOptional(HttpStatusCode::RequestEntityTooLarge);
        case 414: return infra::MakeOptional(HttpStatusCode::RequestUriTooLarge);
        case 415: return infra::MakeOptional(HttpStatusCode::UnsupportedMediaType);
        case 416: return infra::MakeOptional(HttpStatusCode::RequestRangeNotSatisfiable);
        case 417: return infra::MakeOptional(HttpStatusCode::ExpectationFailed);
        case 500: return infra::MakeOptional(HttpStatusCode::InternalServerError);
        case 501: return infra::MakeOptional(HttpStatusCode::NotImplemented);
        case 502: return infra::MakeOptional(HttpStatusCode::BadGateway);
        case 503: return infra::MakeOptional(HttpStatusCode::ServiceUnavailable);
        case 504: return infra::MakeOptional(HttpStatusCode::GatewayTimeOut);
        case 505: return infra::MakeOptional(HttpStatusCode::HttpVersionNotSupported);
        }

        return infra::none;
    }

    infra::BoundedConstString HttpStatusCodeToString(services::HttpStatusCode statusCode)
    {
        switch (statusCode)
        {
        case services::HttpStatusCode::Continue: return "Continue";
        case services::HttpStatusCode::SwitchingProtocols: return "SwitchingProtocols";
        case services::HttpStatusCode::OK: return "OK";
        case services::HttpStatusCode::Created: return "Created";
        case services::HttpStatusCode::Accepted: return "Accepted";
        case services::HttpStatusCode::NonAuthorativeInformation: return "NonAuthorativeInformation";
        case services::HttpStatusCode::NoContent: return "NoContent";
        case services::HttpStatusCode::ResetContent: return "ResetContent";
        case services::HttpStatusCode::PartialContent: return "PartialContent";
        case services::HttpStatusCode::MultipleChoices: return "MultipleChoices";
        case services::HttpStatusCode::MovedPermanently: return "MovedPermanently";
        case services::HttpStatusCode::Found: return "Found";
        case services::HttpStatusCode::SeeOther: return "SeeOther";
        case services::HttpStatusCode::NotModified: return "NotModified";
        case services::HttpStatusCode::UseProxy: return "UseProxy";
        case services::HttpStatusCode::TemporaryRedirect: return "TemporaryRedirect";
        case services::HttpStatusCode::BadRequest: return "BadRequest";
        case services::HttpStatusCode::Unauthorized: return "Unauthorized";
        case services::HttpStatusCode::PaymentRequired: return "PaymentRequired";
        case services::HttpStatusCode::Forbidden: return "Forbidden";
        case services::HttpStatusCode::NotFound: return "NotFound";
        case services::HttpStatusCode::MethodNotAllowed: return "MethodNotAllowed";
        case services::HttpStatusCode::NotAcceptable: return "NotAcceptable";
        case services::HttpStatusCode::ProxyAuthenticationRequired: return "ProxyAuthenticationRequired";
        case services::HttpStatusCode::RequestTimeOut: return "RequestTimeOut";
        case services::HttpStatusCode::Conflict: return "Conflict";
        case services::HttpStatusCode::Gone: return "Gone";
        case services::HttpStatusCode::LengthRequired: return "LengthRequired";
        case services::HttpStatusCode::PreconditionFailed: return "PreconditionFailed";
        case services::HttpStatusCode::RequestEntityTooLarge: return "RequestEntityTooLarge";
        case services::HttpStatusCode::RequestUriTooLarge: return "RequestUriTooLarge";
        case services::HttpStatusCode::UnsupportedMediaType: return "UnsupportedMediaType";
        case services::HttpStatusCode::RequestRangeNotSatisfiable: return "RequestRangeNotSatisfiable";
        case services::HttpStatusCode::ExpectationFailed: return "ExpectationFailed";
        case services::HttpStatusCode::InternalServerError: return "InternalServerError";
        case services::HttpStatusCode::NotImplemented: return "NotImplemented";
        case services::HttpStatusCode::BadGateway: return "BadGateway";
        case services::HttpStatusCode::ServiceUnavailable: return "ServiceUnavailable";
        case services::HttpStatusCode::GatewayTimeOut: return "GatewayTimeOut";
        case services::HttpStatusCode::HttpVersionNotSupported: return "HttpVersionNotSupported";
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
