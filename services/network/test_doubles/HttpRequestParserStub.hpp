#ifndef NETWORK_HTTP_REQUEST_PARSER_STUB_HPP
#define NETWORK_HTTP_REQUEST_PARSER_STUB_HPP

#include "services/network/HttpRequestParser.hpp"
#include "gmock/gmock.h"
#include <string>
#include <utility>
#include <vector>

namespace services
{
    class HttpRequestParserStub
        : public HttpRequestParser
    {
    public:
        HttpRequestParserStub(HttpVerb verb, const std::string& body)
            : verb(verb)
            , pathTokens(path, '/')
            , body(body)
            , contentLength(body.size())
        {}

        HttpRequestParserStub(HttpVerb verb, infra::BoundedConstString path, const std::string& body)
            : verb(verb)
            , path(path)
            , pathTokens(path, '/')
            , body(body)
            , contentLength(body.size())
        {}

        virtual bool HeadersComplete() const override
        {
            return true;
        }

        virtual bool Valid() const override
        {
            return true;
        }

        virtual HttpVerb Verb() const override
        {
            return verb;
        }

        virtual const infra::Tokenizer& PathTokens() const override
        {
            return pathTokens;
        }

        virtual infra::BoundedConstString Header(infra::BoundedConstString name) const override
        {
            for (auto& header : headers)
                if (header.first == name)
                    return header.second;

            return infra::BoundedConstString();
        }

        virtual void EnumerateHeaders(const infra::Function<void(infra::BoundedConstString name, infra::BoundedConstString value)>& enumerator) const override
        {
            for (auto& header : headers)
                enumerator(header.first, header.second);
        }

        virtual infra::BoundedString& BodyBuffer() override
        {
            return body;
        }

        virtual std::optional<uint32_t> ContentLength() const
        {
            return std::make_optional(contentLength);
        }

        HttpVerb verb;
        infra::BoundedConstString path;
        infra::Tokenizer pathTokens;
        infra::BoundedString::WithStorage<1024> body;
        uint32_t contentLength;
        std::vector<std::pair<std::string, std::string>> headers;
    };
}

#endif
