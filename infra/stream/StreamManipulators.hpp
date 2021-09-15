#ifndef INFRA_STREAM_MANIPULATORS_HPP
#define INFRA_STREAM_MANIPULATORS_HPP

#include <cstdlib>

namespace infra
{
    extern const struct Text
    {
    } text;
    extern const struct Hex
    {
    } hex;
    extern const struct Bin
    {
    } bin;
    extern const struct Data
    {
    } data;
    extern const struct Endl
    {
    } endl;

    struct Width
    {
        explicit Width(std::size_t width, char padding = ' ');

        std::size_t width;
        char padding;
    };
    extern const Width resetWidth;
}

#endif
