#ifndef INFRA_TOKENIZER_HPP
#define INFRA_TOKENIZER_HPP

#include "infra/util/BoundedString.hpp"

namespace infra
{
    class Tokenizer
    {
    public:
        Tokenizer(infra::BoundedConstString string, char separator);

        infra::BoundedConstString Token(std::size_t tokenIndex) const;
        infra::BoundedConstString TokenAndRest(std::size_t tokenIndex) const;
        std::size_t Size() const;

    private:
        infra::BoundedConstString MakeToken(std::size_t start) const;
        std::size_t SkipTokens(std::size_t tokenIndex) const;
        std::size_t StartOfNextToken(std::size_t start) const;
        std::size_t SkipConsecutiveSeparators(std::size_t index) const;

    private:
        infra::BoundedConstString string;
        char separator;
    };
}

#endif
