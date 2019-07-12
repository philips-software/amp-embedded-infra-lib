#include "infra/util/Tokenizer.hpp"

namespace infra
{
    Tokenizer::Tokenizer(infra::BoundedConstString string, char separator)
        : string(string)
        , separator(separator)
    {}

    infra::BoundedConstString Tokenizer::Token(std::size_t tokenIndex) const
    {
        std::size_t start = SkipTokens(tokenIndex);

        return MakeToken(start);
    }

    infra::BoundedConstString Tokenizer::TokenAndRest(std::size_t tokenIndex) const
    {
        std::size_t start = SkipTokens(tokenIndex);

        if (start != infra::BoundedConstString::npos)
            return infra::BoundedConstString(string.begin() + start, string.size() - start);
        else
            return infra::BoundedConstString();
    }

    std::size_t Tokenizer::Size() const
    {
        std::size_t result = 0;
        std::size_t start = SkipConsecutiveSeparators(0);

        while (start != infra::BoundedConstString::npos)
        {
            ++result;
            start = StartOfNextToken(start);
        }

        return result;
    }

    infra::BoundedConstString Tokenizer::MakeToken(std::size_t start) const
    {
        if (start < string.size())
            return string.substr(start, string.find(separator, start) - start);
        else
            return infra::BoundedConstString();
    }

    std::size_t Tokenizer::SkipTokens(std::size_t tokenIndex) const
    {
        std::size_t start = SkipConsecutiveSeparators(0);

        while (tokenIndex-- != 0 && start != infra::BoundedConstString::npos)
            start = StartOfNextToken(start);

        return start;
    }

    std::size_t Tokenizer::StartOfNextToken(std::size_t start) const
    {
        return SkipConsecutiveSeparators(string.find(separator, start));
    }

    std::size_t Tokenizer::SkipConsecutiveSeparators(std::size_t index) const
    {
        while (index < string.size() && string[index] == separator)
            ++index;

        if (index == string.size())
            return infra::BoundedConstString::npos;
        else
            return index;
    }
}
