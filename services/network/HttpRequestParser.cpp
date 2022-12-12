#include "services/network/HttpRequestParser.hpp"
#include "infra/stream/StringInputStream.hpp"

namespace services
{
    HttpRequestParserImpl::HttpRequestParserImpl(infra::BoundedString& data)
        : pathTokens("", '/')
    {
        infra::Tokenizer tokenizer(data, ' ');

        FindVerb(tokenizer);
        FindPath(tokenizer);
        FindHeadersAndBodyStart(data);
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

    bool HttpRequestParserImpl::HeadersComplete() const
    {
        return headersComplete;
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

    infra::BoundedString& HttpRequestParserImpl::BodyBuffer()
    {
        return bodyBuffer;
    }

    infra::Optional<uint32_t> HttpRequestParserImpl::ContentLength() const
    {
        return contentLength;
    }

    void HttpRequestParserImpl::SetContentLength(uint32_t length)
    {
        contentLength = length;
    }

    void HttpRequestParserImpl::FindHeadersAndBodyStart(infra::BoundedString& data)
    {
        auto bodyStart = data.find("\r\n\r\n", 0, 4);
        if (bodyStart == infra::BoundedString::npos)
            headersComplete = false;
        else
        {
            auto headersStart = data.find("\r\n");
            if (headersStart != bodyStart)
                headers = data.substr(headersStart + 2, bodyStart - headersStart - 2);

            bodyStart += 4;

            headersComplete = true;

            data.resize(data.max_size());
            bodyBuffer = data.substr(bodyStart);
            bodyBuffer.clear();
            data.resize(bodyStart);

            ReadContentLength();
        }
    }

    void HttpRequestParserImpl::ReadContentLength()
    {
        auto contentsLengthString = Header("Content-Length");
        if (!contentsLengthString.empty())
        {
            infra::StringInputStream stream(contentsLengthString, infra::noFail);
            contentLength.Emplace(0);
            stream >> *contentLength;
        }
        else if (verb == HttpVerb::get || verb == HttpVerb::head || verb == HttpVerb::delete_ || verb == HttpVerb::connect)
            contentLength = 0;
    }
}
