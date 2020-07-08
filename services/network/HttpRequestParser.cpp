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
        auto receivedVerb = HttpVerbFromString(verbToken);

        if (receivedVerb)
            verb = *receivedVerb;
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

    HttpVerb HttpRequestParserImpl::Verb() const
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
            if (infra::CaseInsensitiveCompare(tokenizer.Token(0), name))
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

            auto contentsLength = Header("Content-Length");
            if (!contentsLength.empty())
            {
                infra::StringInputStream stream(contentsLength, infra::noFail);
                uint32_t size(0);
                stream >> size;

                complete = body.size() >= size;
            }
        }
    }
}
