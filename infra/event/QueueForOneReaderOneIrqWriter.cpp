#include "infra/event/EventDispatcher.hpp"
#include "infra/event/QueueForOneReaderOneIrqWriter.hpp"
#include <cassert>

namespace infra
{
    QueueForOneReaderOneIrqWriter::QueueForOneReaderOneIrqWriter(const infra::ByteRange& buffer, const infra::Function<void()>& onDataAvailable)
        : buffer(buffer)
        , contentsBegin(buffer.begin())
        , contentsEnd(buffer.begin())
        , onDataAvailable(onDataAvailable)
    {
        notificationScheduled = false;
    }

    void QueueForOneReaderOneIrqWriter::AddFromInterrupt(uint8_t element)
    {
        assert(!Full());
        *contentsEnd = element;

        if (contentsEnd == buffer.end() - 1)
            contentsEnd = buffer.begin();
        else
            ++contentsEnd;
       
        NotifyDataAvailable();
    }

    void QueueForOneReaderOneIrqWriter::AddFromInterrupt(infra::ConstByteRange data)
    {
        std::size_t copySize = std::min<std::size_t>(data.size(), buffer.end() - contentsEnd);
        uint8_t* begin = contentsBegin.load();
        if (!(begin <= contentsEnd.load() || begin > contentsEnd.load() + copySize))
            return;

        std::copy(data.begin(), data.begin() + copySize, contentsEnd.load());

        if (contentsEnd == buffer.end() - copySize)
            contentsEnd = buffer.begin();
        else
            contentsEnd += copySize;

        data.pop_front(copySize);

        assert(begin <= contentsEnd.load() || begin > contentsEnd.load() + data.size());
        std::copy(data.begin(), data.end(), contentsEnd.load());
        contentsEnd += data.size();

        NotifyDataAvailable();
    }

    bool QueueForOneReaderOneIrqWriter::Empty() const
    {
        return contentsBegin.load() == contentsEnd.load();
    }

    bool QueueForOneReaderOneIrqWriter::Full() const
    {
        const uint8_t* begin = contentsBegin.load();
        const uint8_t* end = contentsEnd.load();

        return Full(begin, end);
    }

    bool QueueForOneReaderOneIrqWriter::Full(const uint8_t* begin, const uint8_t* end) const
    {
        return (end == buffer.end() - 1 || begin == end + 1)
            && (end != buffer.end() - 1 || begin == buffer.begin());
    }

    uint8_t QueueForOneReaderOneIrqWriter::Get()
    {
        assert(!Empty());
        uint8_t result = *contentsBegin;

        if (contentsBegin == buffer.end() - 1)
            contentsBegin = buffer.begin();
        else
            ++contentsBegin;

        return result;
    }

    uint8_t QueueForOneReaderOneIrqWriter::operator [] (size_t position) const
    {
        size_t size = Size();
        assert(size > 0);
        assert(position < size);

        uint8_t* begin = contentsBegin;
        if (begin + position >= buffer.end())
        {
            position -= buffer.end() - begin;
            begin = buffer.begin();
        }

        begin += position;

        return *begin;
    }

    infra::ConstByteRange QueueForOneReaderOneIrqWriter::ContiguousRange(uint32_t offset) const
    {
        const uint8_t* end = contentsEnd.load();
        uint8_t* begin = contentsBegin;

        if (begin + offset >= buffer.end())
        {
            offset -= buffer.end() - begin;
            begin = buffer.begin();
        }

        begin += offset;

        if (end < begin)
            return infra::ConstByteRange(begin, buffer.end());
        else
            return infra::ConstByteRange(begin, end);
    }

    void QueueForOneReaderOneIrqWriter::Consume(uint32_t amount)
    {
        if (contentsBegin + amount >= buffer.end())
        {
            amount -= buffer.end() - contentsBegin;
            contentsBegin = buffer.begin();
        }

        contentsBegin += amount;
    }

    std::size_t QueueForOneReaderOneIrqWriter::Size() const
    {
        const uint8_t* begin = contentsBegin.load();
        const uint8_t* end = contentsEnd.load();

        if (Full(begin, end))
            return EmptySize();
        else if (end >= begin)
            return end - begin;

        return (buffer.end() - begin) + (end - buffer.begin());
    }

    std::size_t QueueForOneReaderOneIrqWriter::EmptySize() const
    {
        return buffer.size() - 1;
    }

    void QueueForOneReaderOneIrqWriter::NotifyDataAvailable()
    {        
        if (!Empty() && !notificationScheduled.exchange(true))
            infra::EventDispatcher::Instance().Schedule([this]() { DataAvailable(); });
    }

    void QueueForOneReaderOneIrqWriter::DataAvailable()
    {
        notificationScheduled = false;
        onDataAvailable();
    }
}
