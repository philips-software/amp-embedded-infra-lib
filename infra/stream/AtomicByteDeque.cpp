#include "infra/stream/AtomicByteDeque.hpp"

namespace infra
{
    AtomicByteDeque::AtomicByteDeque(infra::ByteRange storage)
        : storage(storage)
    {}

    void AtomicByteDeque::Push(infra::ConstByteRange range)
    {
        assert(range.size() <= MaxSize() - Size());

        while (!range.empty())
        {
            auto to = infra::Head(infra::ByteRange(e, storage.end()), range.size());
            infra::Copy(infra::Head(range, to.size()), to);
            range.pop_front(to.size());
            e += to.size();
            if (e == storage.end())
                e = storage.begin();
        }

        assert(storage.begin() <= e && e < storage.end());
        assert(storage.begin() <= b && b <= storage.end());
    }

    void AtomicByteDeque::Pop(std::size_t size)
    {
        auto takenFromEnd = std::min<std::size_t>(size, storage.end() - b);
        b += takenFromEnd;

        assert(storage.begin() <= b && b <= storage.end());
        assert(storage.begin() <= e && e <= storage.end());

        if (b == storage.end())
            b = storage.begin() + size - takenFromEnd;

        assert(storage.begin() <= b && b < storage.end());
        assert(storage.begin() <= e && e <= storage.end());
    }

    std::size_t AtomicByteDeque::Size() const
    {
        const uint8_t* begin = b;
        const uint8_t* end = e;

        if (begin <= end)
            return end - begin;
        else
            return storage.size() + (end - begin);
    }

    std::size_t AtomicByteDeque::MaxSize() const
    {
        return storage.size() - 1;
    }

    bool AtomicByteDeque::Empty() const
    {
        auto end = e.load();
        auto begin = b.load();
        return begin == end;
    }

    infra::ConstByteRange AtomicByteDeque::PeekContiguousRange(std::size_t start) const
    {
        auto end = e.load();
        auto begin = b.load();

        auto takenFromEnd = std::min<std::size_t>(start, storage.end() - begin);
        begin += takenFromEnd;

        if (begin == storage.end())
            begin = storage.begin() + start - takenFromEnd;

        if (begin <= end)
            return infra::ConstByteRange(begin, end);
        else
            return infra::ConstByteRange(begin, storage.end());
    }

    AtomicByteDequeReader::AtomicByteDequeReader(AtomicByteDeque& deque)
        : deque(deque)
    {}

    void AtomicByteDequeReader::Commit()
    {
        deque.Pop(offset);
        offset = 0;
    }

    void AtomicByteDequeReader::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        while (!range.empty())
        {
            auto dequeRange = infra::Head(PeekContiguousRange(0), range.size());
            errorPolicy.ReportResult(!dequeRange.empty());
            if (dequeRange.empty())
                break;
            infra::Copy(dequeRange, infra::Head(range, dequeRange.size()));
            range.pop_front(dequeRange.size());
            offset += dequeRange.size();
        }
    }

    uint8_t AtomicByteDequeReader::Peek(infra::StreamErrorPolicy& errorPolicy)
    {
        auto range = PeekContiguousRange(0);
        errorPolicy.ReportResult(!range.empty());
        if (!range.empty())
            return range.front();
        else
            return 0;
    }

    infra::ConstByteRange AtomicByteDequeReader::ExtractContiguousRange(std::size_t max)
    {
        auto range = infra::Head(PeekContiguousRange(0), max);
        offset += range.size();
        return range;
    }

    infra::ConstByteRange AtomicByteDequeReader::PeekContiguousRange(std::size_t start)
    {
        return deque.PeekContiguousRange(start + offset);
    }

    bool AtomicByteDequeReader::Empty() const
    {
        return deque.Size() == offset;
    }

    std::size_t AtomicByteDequeReader::Available() const
    {
        return deque.Size() - offset;
    }

    std::size_t AtomicByteDequeReader::ConstructSaveMarker() const
    {
        return offset;
    }

    void AtomicByteDequeReader::Rewind(std::size_t marker)
    {
        offset = marker;
    }

    AtomicByteDequeWriter::AtomicByteDequeWriter(AtomicByteDeque& deque)
        : deque(deque)
    {}

    void AtomicByteDequeWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        auto available = Available();
        errorPolicy.ReportResult(range.size() <= available);
        deque.Push(infra::Head(range, available));
    }

    std::size_t AtomicByteDequeWriter::Available() const
    {
        return deque.MaxSize() - deque.Size();
    }

    std::size_t AtomicByteDequeWriter::ConstructSaveMarker() const
    {
        std::abort();
    }

    std::size_t AtomicByteDequeWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        std::abort();
    }

    infra::ByteRange AtomicByteDequeWriter::SaveState(std::size_t marker)
    {
        std::abort();
    }

    void AtomicByteDequeWriter::RestoreState(infra::ByteRange range)
    {
        std::abort();
    }

    infra::ByteRange AtomicByteDequeWriter::Overwrite(std::size_t marker)
    {
        std::abort();
    }
}
