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
        {}

        HttpRequestParserStub(HttpVerb verb, infra::BoundedConstString path, const std::string& body)
            : verb(verb)
            , path(path)
            , pathTokens(path, '/')
            , body(body)
        {}

        virtual bool Complete() const override { return true; }
        virtual bool Valid() const override { return true; }
        virtual HttpVerb Verb() const override { return verb; }
        virtual const infra::Tokenizer& PathTokens() const override { return pathTokens; }
        virtual infra::BoundedString Body() override { return body; }
        virtual infra::BoundedConstString Body() const override { return body; }
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

        HttpVerb verb;
        infra::BoundedConstString path;
        infra::Tokenizer pathTokens;
        std::string body;
        std::vector<std::pair<std::string, std::string>> headers;
    };
}

#endif
