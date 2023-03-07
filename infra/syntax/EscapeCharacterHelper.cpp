#include "infra/syntax/EscapeCharacterHelper.hpp"

namespace infra
{
    std::tuple<std::size_t, infra::BoundedConstString> NonEscapedSubString(infra::BoundedConstString string, std::size_t start, const char* charactersToEscape)
    {
        std::size_t escape = std::min(string.find_first_of(charactersToEscape, start), string.size());
        infra::BoundedConstString nonEscapedSubString = string.substr(start, escape - start);

        for (std::size_t control = start; control != escape; ++control)
            if (string[control] < 0x20)
            {
                escape = control;
                nonEscapedSubString = string.substr(start, escape - start);
                break;
            }

        return { escape, nonEscapedSubString };
    }

    void InsertEscapedContent(infra::TextOutputStream& stream, infra::BoundedConstString content, const char* charactersToEscape, infra::Function<void(infra::TextOutputStream&, char)> replaceEscapeCharacter)
    {
        std::size_t start = 0;
        while (start != content.size())
        {
            auto [escape, nonEscapedSubString] = NonEscapedSubString(content, start, charactersToEscape);

            start = escape;
            if (!nonEscapedSubString.empty())
                stream << nonEscapedSubString;
            if (escape != content.size())
            {
                replaceEscapeCharacter(stream, content[escape]);
                ++start;
            }
        }
    }
}
