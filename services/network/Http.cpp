#include "services/network/Http.hpp"

namespace services
{
	namespace
	{
		static const char* separator = ":";

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
		}

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
}

namespace infra
{
    TextOutputStream& operator<<(TextOutputStream& stream, const services::HttpHeader& header)
    {
        stream << header.Field() << services::separator << header.Value();

        return stream;
    }

	TextOutputStream& operator<<(TextOutputStream& stream, services::HttpStatusCode statusCode)
	{
		stream << services::HttpStatusCodeToString(statusCode);

		return stream;
	}
}
