#include "protobuf/echo/BufferingStreamWriter.hpp"

namespace services
{
    BufferingStreamWriter::BufferingStreamWriter(infra::BoundedDeque<uint8_t>& buffer, infra::StreamWriter& output)
        : buffer(buffer)
        , output(output)
    {
        LoadRemainder();
    }

    void BufferingStreamWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        if (index < output.Available())
        {
            auto first = infra::Head(range, output.Available() - index);
            output.Insert(first, errorPolicy);
            index += first.size();
            range.pop_front(first.size());
        }

        buffer.insert(buffer.end(), range.begin(), range.end());
        index += range.size();
    }

    std::size_t BufferingStreamWriter::Available() const
    {
        return output.Available() + buffer.size() - index;
    }

    std::size_t BufferingStreamWriter::ConstructSaveMarker() const
    {
        return index;
    }

    std::size_t BufferingStreamWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        return index - marker;
    }

    infra::ByteRange BufferingStreamWriter::SaveState(std::size_t marker)
    {
        std::abort();
    }

    void BufferingStreamWriter::RestoreState(infra::ByteRange range)
    {
        std::abort();
    }

    infra::ByteRange BufferingStreamWriter::Overwrite(std::size_t marker)
    {
        std::abort();
    }

    void BufferingStreamWriter::LoadRemainder()
    {
        infra::StreamErrorPolicy errorPolicy;
        auto from = infra::Head(buffer.contiguous_range(buffer.begin()), output.Available());
        output.Insert(from, errorPolicy);
        buffer.erase(buffer.begin(), buffer.begin() + from.size());
        from = infra::Head(buffer.contiguous_range(buffer.begin()), output.Available());
        output.Insert(from, errorPolicy);
        buffer.clear();

        index = 0;
    }
}
