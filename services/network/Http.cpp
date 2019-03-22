#include "services/network/Http.hpp"

namespace {
    static const char* separator = ":";
}

namespace services
{
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

    infra::BoundedConstString HostFromUrl(infra::BoundedConstString url)
    {
        auto schemeEnd = url.find("//");
        if (schemeEnd == infra::BoundedString::npos)
            schemeEnd = 0;
        else
            schemeEnd += 2;

        return url.substr(schemeEnd, url.find('/', schemeEnd) - schemeEnd);
    }

    infra::BoundedConstString PathFromUrl(infra::BoundedConstString url)
    {
        auto schemeEnd = url.find("//");
        if (schemeEnd == infra::BoundedString::npos)
            schemeEnd = 0;
        else
            schemeEnd += 2;

        auto separator = url.find('/', schemeEnd);
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
        stream << header.Field() << separator << header.Value();

        return stream;
    }
}
