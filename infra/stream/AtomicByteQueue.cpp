#include "infra/stream/AtomicByteQueue.hpp"

namespace infra
{
    AtomicByteQueue::AtomicByteQueue(infra::ByteRange storage)
        : storage(storage)
    {}

    void AtomicByteQueue::Push(infra::ConstByteRange range)
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

    void AtomicByteQueue::Pop(std::size_t size)
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

    std::size_t AtomicByteQueue::Size() const
    {
        const uint8_t* begin = b;
        const uint8_t* end = e;

        if (begin <= end)
            return end - begin;
        else
            return storage.size() + (end - begin);
    }

    std::size_t AtomicByteQueue::MaxSize() const
    {
        return storage.size() - 1;
    }

    bool AtomicByteQueue::Empty() const
    {
        auto end = e.load();
        auto begin = b.load();
        return begin == end;
    }

    infra::ConstByteRange AtomicByteQueue::PeekContiguousRange(std::size_t start) const
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

    AtomicByteQueueReader::AtomicByteQueueReader(AtomicByteQueue& deque)
        : deque(deque)
    {}

    void AtomicByteQueueReader::Commit()
    {
        deque.Pop(offset);
        offset = 0;
    }

    void AtomicByteQueueReader::Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy)
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

    uint8_t AtomicByteQueueReader::Peek(infra::StreamErrorPolicy& errorPolicy)
    {
        auto range = PeekContiguousRange(0);
        errorPolicy.ReportResult(!range.empty());
        if (!range.empty())
            return range.front();
        else
            return 0;
    }

    infra::ConstByteRange AtomicByteQueueReader::ExtractContiguousRange(std::size_t max)
    {
        auto range = infra::Head(PeekContiguousRange(0), max);
        offset += range.size();
        return range;
    }

    infra::ConstByteRange AtomicByteQueueReader::PeekContiguousRange(std::size_t start)
    {
        return deque.PeekContiguousRange(start + offset);
    }

    bool AtomicByteQueueReader::Empty() const
    {
        return deque.Size() == offset;
    }

    std::size_t AtomicByteQueueReader::Available() const
    {
        return deque.Size() - offset;
    }

    std::size_t AtomicByteQueueReader::ConstructSaveMarker() const
    {
        return offset;
    }

    void AtomicByteQueueReader::Rewind(std::size_t marker)
    {
        offset = marker;
    }

    AtomicByteQueueWriter::AtomicByteQueueWriter(AtomicByteQueue& deque)
        : deque(deque)
    {}

    void AtomicByteQueueWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        auto available = Available();
        errorPolicy.ReportResult(range.size() <= available);
        deque.Push(infra::Head(range, available));
    }

    std::size_t AtomicByteQueueWriter::Available() const
    {
        return deque.MaxSize() - deque.Size();
    }

    std::size_t AtomicByteQueueWriter::ConstructSaveMarker() const
    {
        std::abort();
    }

    std::size_t AtomicByteQueueWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        std::abort();
    }

    infra::ByteRange AtomicByteQueueWriter::SaveState(std::size_t marker)
    {
        std::abort();
    }

    void AtomicByteQueueWriter::RestoreState(infra::ByteRange range)
    {
        std::abort();
    }

    infra::ByteRange AtomicByteQueueWriter::Overwrite(std::size_t marker)
    {
        std::abort();
    }
}
