#include "infra/util/BoundedString.hpp"

namespace infra
{
    BoundedString ByteRangeAsString(infra::MemoryRange<uint8_t> range)
    {
        return BoundedString(reinterpret_cast<char*>(range.begin()), range.size());
    }

    BoundedConstString ByteRangeAsString(infra::MemoryRange<const uint8_t> range)
    {
        return BoundedConstString(reinterpret_cast<const char*>(range.begin()), range.size());
    }

    MemoryRange<uint8_t> StdStringAsByteRange(std::string& string)
    {
        return StringAsMemoryRange<uint8_t>(infra::BoundedString(string));
    }

    MemoryRange<const uint8_t> StdStringAsByteRange(const std::string& string)
    {
        return StringAsMemoryRange<uint8_t>(infra::BoundedConstString(string));
    }
}
