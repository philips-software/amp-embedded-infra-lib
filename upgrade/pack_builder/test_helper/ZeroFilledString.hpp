#ifndef UPGRADE_PACK_BUILDER_ZERO_FILLED_STRING_HPP
#define UPGRADE_PACK_BUILDER_ZERO_FILLED_STRING_HPP

#include <algorithm>
#include <cassert>
#include <string>

class ZeroFilledString
{
public:
    ZeroFilledString(std::size_t length, const std::string& contents)
        : length(length)
        , contents(contents)
    {
        assert(contents.size() <= length);
    }

    bool operator==(const char* other) const
    {
        return std::equal(contents.begin(), contents.end(), other) && std::all_of(other + contents.size(), other + length, [](char i)
                                                                          { return i == 0; });
    }

private:
    std::size_t length;
    std::string contents;
};

#endif
