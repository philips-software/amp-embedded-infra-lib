#include "infra/stream/ByteInputStream.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/BoundedVector.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace
{
    // Bounds the work one input can ask for, so a fuzz case that is merely large does
    // not read as a hang. Deep nesting is the interesting part, not deep recursion.
    constexpr uint32_t maxFields = 64;
    constexpr uint32_t maxDepth = 4;

    void ParseFields(infra::ProtoParser& parser, uint32_t depth)
    {
        for (uint32_t field = 0; field != maxFields && !parser.Empty() && !parser.FormatFailed(); ++field)
        {
            auto [value, fieldNumber] = parser.GetField();

            if (std::holds_alternative<infra::ProtoLengthDelimited>(value))
            {
                auto& delimited = std::get<infra::ProtoLengthDelimited>(value);

                // Drive the accessors a generated message would use. Each one reads the
                // same bytes through a different bound, which is where a length that
                // outruns the stream would show up.
                infra::BoundedString::WithStorage<16> string;
                delimited.GetString(string);

                infra::BoundedConstString stringReference;
                delimited.GetStringReference(stringReference);

                infra::BoundedVector<uint8_t>::WithMaxSize<16> bytes;
                delimited.GetBytes(bytes);

                infra::ConstByteRange bytesReference;
                delimited.GetBytesReference(bytesReference);

                if (depth != maxDepth)
                {
                    auto nested = delimited.Parser();
                    ParseFields(nested, depth + 1);
                }

                delimited.SkipEverything();
            }
        }
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, std::size_t size)
{
    infra::ByteInputStream stream(infra::ConstByteRange(data, data + size), infra::softFail);
    infra::ProtoParser parser(stream);

    ParseFields(parser, 0);

    // softFail requires the caller to read the result before the policy is destroyed.
    static_cast<void>(parser.FormatFailed());
    static_cast<void>(stream.ErrorPolicy().Failed());

    return 0;
}
