#include "infra/util/BoundedString.hpp"

namespace infra
{

    namespace literals
    {
        infra::BoundedConstString operator""_s(const char* str, std::size_t count)
        {
            return infra::BoundedConstString(str, count);
        }
    }

    BoundedString ByteRangeAsString(infra::MemoryRange<uint8_t> range)
    {
        return BoundedString(reinterpret_cast<char*>(range.begin()), range.size());
    }

    BoundedConstString ByteRangeAsString(infra::MemoryRange<const uint8_t> range)
    {
        return BoundedConstString(reinterpret_cast<const char*>(range.begin()), range.size());
    }

    MemoryRange<const uint8_t> StdStringAsByteRange(const std::string& string)
    {
        return StringAsMemoryRange<uint8_t>(infra::BoundedConstString(string));
    }
}
