#ifndef SERVICES_HTTP_REQUEST_PARSER_HPP
#define SERVICES_HTTP_REQUEST_PARSER_HPP

#include "infra/util/BoundedString.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Tokenizer.hpp"
#include "services/network/Http.hpp"

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
        virtual bool HeadersComplete() const = 0;
        virtual bool Valid() const = 0;
        virtual HttpVerb Verb() const = 0;
        virtual const infra::Tokenizer& PathTokens() const = 0;
        virtual infra::BoundedConstString Header(infra::BoundedConstString name) const = 0;
        virtual void EnumerateHeaders(const infra::Function<void(infra::BoundedConstString name, infra::BoundedConstString value)>& enumerator) const = 0;
        virtual infra::BoundedString& BodyBuffer() = 0;
        virtual std::optional<uint32_t> ContentLength() const = 0;
    };

    class HttpRequestParserImpl
        : public HttpRequestParser
    {
    public:
        explicit HttpRequestParserImpl(infra::BoundedString& data);

        bool HeadersComplete() const override;
        bool Valid() const override;
        HttpVerb Verb() const override;
        const infra::Tokenizer& PathTokens() const override;
        infra::BoundedConstString Header(infra::BoundedConstString name) const override;
        void EnumerateHeaders(const infra::Function<void(infra::BoundedConstString name, infra::BoundedConstString value)>& enumerator) const override;
        infra::BoundedString& BodyBuffer() override;
        std::optional<uint32_t> ContentLength() const override;

        void SetContentLength(uint32_t length);

    private:
        void FindVerb(infra::Tokenizer& tokenizer);
        void FindPath(infra::Tokenizer& tokenizer);
        void FindHeadersAndBodyStart(infra::BoundedString& data);
        void ReadContentLength();

    private:
        infra::BoundedString headers;
        infra::BoundedString bodyBuffer;
        HttpVerb verb;
        infra::BoundedConstString path;
        infra::Tokenizer pathTokens;
        bool headersComplete = true;
        bool valid = true;
        std::optional<uint32_t> contentLength;
    };
}

#endif
