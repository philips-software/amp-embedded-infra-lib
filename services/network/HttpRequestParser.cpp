#include "infra/stream/StringInputStream.hpp"
#include "services/network/HttpRequestParser.hpp"

namespace services
{
    HttpRequestParserImpl::HttpRequestParserImpl(infra::BoundedString data)
        : pathTokens("", '/')
    {
        infra::Tokenizer tokenizer(data, ' ');

        FindVerb(tokenizer);
        FindPath(tokenizer);
        FindHeadersAndBody(data);
    }

    void HttpRequestParserImpl::FindVerb(infra::Tokenizer& tokenizer)
    {
        auto verbToken = tokenizer.Token(0);

        if (verbToken == "PUT")
            verb = HttpVerb::put;
        else if (verbToken == "GET")
            verb = HttpVerb::get;
        else if (verbToken == "DELETE")
            verb = HttpVerb::delete_;
        else if (verbToken == "POST")
            verb = HttpVerb::post;
        else
            valid = false;
    }

    void HttpRequestParserImpl::FindPath(infra::Tokenizer& tokenizer)
    {
        path = tokenizer.Token(1);
        pathTokens = infra::Tokenizer(path, '/');
    }

    bool HttpRequestParserImpl::Complete() const
    {
        return complete;
    }

    bool HttpRequestParserImpl::Valid() const
    {
        return valid;
    }

    HttpRequestParserImpl::HttpVerb HttpRequestParserImpl::Verb() const
    {
        return verb;
    }

    const infra::Tokenizer& HttpRequestParserImpl::PathTokens() const
    {
        return pathTokens;
    }

    infra::BoundedString HttpRequestParserImpl::Body()
    {
        return body;
    }

    infra::BoundedConstString HttpRequestParserImpl::Body() const
    {
        return body;
    }

    infra::BoundedConstString HttpRequestParserImpl::Header(infra::BoundedConstString name) const
    {
        auto headersTail = headers;

        while (!headersTail.empty())
        {
            auto nextHeaderStart = headersTail.find("\r\n");
            infra::Tokenizer tokenizer(headersTail.substr(0, nextHeaderStart), ':');
            if (tokenizer.Token(0) == name)
                return infra::TrimLeft(tokenizer.TokenAndRest(1));
            if (nextHeaderStart == infra::BoundedString::npos)
                break;

            headersTail = headersTail.substr(nextHeaderStart + 2);
        }

        return infra::BoundedConstString();
    }

    void HttpRequestParserImpl::EnumerateHeaders(const infra::Function<void(infra::BoundedConstString name, infra::BoundedConstString value)>& enumerator) const
    {
        auto headersTail = headers;

        while (!headersTail.empty())
        {
            auto nextHeaderStart = headersTail.find("\r\n");
            infra::Tokenizer tokenizer(headersTail.substr(0, nextHeaderStart), ':');
            enumerator(tokenizer.Token(0), infra::TrimLeft(tokenizer.TokenAndRest(1)));
            if (nextHeaderStart == infra::BoundedString::npos)
                break;

            headersTail = headersTail.substr(nextHeaderStart + 2);
        }
    }

    void HttpRequestParserImpl::FindHeadersAndBody(infra::BoundedString data)
    {
        auto bodyStart = data.find("\r\n\r\n", 0, 4);
        if (bodyStart == infra::BoundedString::npos)
            complete = false;
        else
        {
            auto headersStart = data.find("\r\n");
            if (headersStart != bodyStart)
                headers = data.substr(headersStart + 2, bodyStart);
            body = data.substr(bodyStart + 4);
        }

        CheckContentsLength(data);
    }

    void HttpRequestParserImpl::CheckContentsLength(infra::BoundedString data)
    {
        static const char* contentLengthTag = "\r\nContent-Length:";
        std::size_t contentLengthHeaderStart = data.find(contentLengthTag);
        if (contentLengthHeaderStart != infra::BoundedConstString::npos)
        {
            auto contentLengthStart = contentLengthHeaderStart + std::strlen(contentLengthTag);
            auto contentLengthEnd = data.find("\r\n", contentLengthStart);
            infra::StringInputStream stream(data.substr(contentLengthStart, contentLengthEnd - contentLengthStart), infra::noFail);
            uint32_t size(0);
            stream >> size;

            if (body.size() < size)
                complete = false;
        }
    }
}
