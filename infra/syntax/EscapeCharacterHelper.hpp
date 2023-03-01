#ifndef INFRA_ESCAPE_CHARACTER_HELPER_HPP
#define INFRA_ESCAPE_CHARACTER_HELPER_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/BoundedString.hpp"
#include <tuple>

namespace infra
{
    std::tuple<std::size_t, infra::BoundedConstString> NonEscapedSubString(infra::BoundedConstString string, std::size_t start, const char* charactersToEscape);
    void InsertEscapedContent(infra::TextOutputStream& stream, infra::BoundedConstString content, const char* charactersToEscape, infra::Function<void(infra::TextOutputStream&, char)> replaceEscapeCharacter);
}

#endif
