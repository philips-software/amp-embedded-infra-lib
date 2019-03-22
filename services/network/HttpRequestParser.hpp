#ifndef DI_COMM_HTTP_REQUEST_PARSER_HPP
#define DI_COMM_HTTP_REQUEST_PARSER_HPP

#include "infra/util/BoundedString.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Tokenizer.hpp"

namespace services
{
    class HttpRequestParser
    {
    protected:
        HttpRequestParser() = default;
        HttpRequestParser(const HttpRequestParser& other) = delete;
        HttpRequestParser& operator=(const HttpRequestParser& other) = delete;
        ~HttpRequestParser() = default;

    public:
        enum class HttpVerb
        {
            get,
            put,
            post,
            delete_
        };

        virtual bool Complete() const = 0;
        virtual bool Valid() const = 0;
        virtual HttpVerb Verb() const = 0;
        virtual const infra::Tokenizer& PathTokens() const = 0;
        virtual infra::BoundedString Body() = 0;
        virtual infra::BoundedConstString Body() const = 0;
        virtual infra::BoundedConstString Header(infra::BoundedConstString name) const = 0;
        virtual void EnumerateHeaders(const infra::Function<void(infra::BoundedConstString name, infra::BoundedConstString value)>& enumerator) const = 0;
    };

    class HttpRequestParserImpl
        : public HttpRequestParser
    {
    public:
        HttpRequestParserImpl(infra::BoundedString data);

        virtual bool Complete() const override;
        virtual bool Valid() const override;
        virtual HttpVerb Verb() const override;
        virtual const infra::Tokenizer& PathTokens() const override;
        virtual infra::BoundedString Body() override;
        virtual infra::BoundedConstString Body() const override;
        virtual infra::BoundedConstString Header(infra::BoundedConstString name) const override;
        virtual void EnumerateHeaders(const infra::Function<void(infra::BoundedConstString name, infra::BoundedConstString value)>& enumerator) const override;

    private:
        void FindVerb(infra::Tokenizer& tokenizer);
        void FindPath(infra::Tokenizer& tokenizer);
        void FindHeadersAndBody(infra::BoundedString data);
        void CheckContentsLength(infra::BoundedString data);

    private:
        infra::BoundedString headers;
        infra::BoundedString body;
        HttpVerb verb;
        infra::BoundedConstString path;
        infra::Tokenizer pathTokens;
        bool complete = true;
        bool valid = true;
    };
}

#endif
