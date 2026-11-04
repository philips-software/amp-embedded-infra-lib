#include "infra/syntax/Asn1.hpp"
#include <cctype>

namespace infra
{
    Asn1Value::Asn1Value(uint8_t tag, infra::ConstByteRange range)
        : tag(tag)
        , range(range)
    {}

    infra::ConstByteRange Asn1Value::Integer() const
    {
        assert(tag == 0x02);

        return range;
    }

    Asn1Sequence Asn1Value::Sequence() const
    {
        assert(tag == 0x30);

        return Asn1Sequence(range);
    }

    Asn1SequenceIterator::Asn1SequenceIterator()
        : currentValue(0, infra::ConstByteRange())
    {}

    Asn1SequenceIterator::Asn1SequenceIterator(infra::ConstByteRange range)
        : range(range)
        , currentValue(0, infra::ConstByteRange())
    {
        ReadCurrentValue();
    }

    bool Asn1SequenceIterator::operator==(const Asn1SequenceIterator& other) const
    {
        return range == other.range;
    }

    bool Asn1SequenceIterator::operator!=(const Asn1SequenceIterator& other) const
    {
        return !(*this == other);
    }

    Asn1Value& Asn1SequenceIterator::operator*()
    {
        return currentValue;
    }

    const Asn1Value& Asn1SequenceIterator::operator*() const
    {
        return currentValue;
    }

    Asn1Value* Asn1SequenceIterator::operator->()
    {
        return &currentValue;
    }

    const Asn1Value* Asn1SequenceIterator::operator->() const
    {
        return &currentValue;
    }

    Asn1SequenceIterator& Asn1SequenceIterator::operator++()
    {
        if (!range.empty())
        {
            range = infra::ConstByteRange(currentValue.range.end(), range.end());
            ReadCurrentValue();
        }

        return *this;
    }

    Asn1SequenceIterator Asn1SequenceIterator::operator++(int)
    {
        Asn1SequenceIterator result(*this);
        ++*this;
        return result;
    }

    void Asn1SequenceIterator::ReadCurrentValue()
    {
        auto value = MakeValue();

        if (value)
            currentValue = *value;
        else
        {
            range = infra::ConstByteRange();
            currentValue = Asn1Value(0, infra::ConstByteRange());
        }
    }

    std::optional<Asn1Value> Asn1SequenceIterator::MakeValue() const
    {
        infra::ConstByteRange range = this->range;
        if (range.size() < 2)
            return std::nullopt;

        uint8_t tag = range.front();
        range.pop_front();

        uint32_t length = range.front();
        range.pop_front();
        if (length == 0x81)
        {
            if (range.empty())
                return std::nullopt;

            length = range.front();
            range.pop_front();
        }
        else if (length == 0x82)
        {
            if (range.size() < 2)
                return std::nullopt;

            length = range.front() << 8;
            range.pop_front();
            length += range.front();
            range.pop_front();
        }
        else if (length > 0x7f)
            return std::nullopt;

        if (length > range.size())
            return std::nullopt;

        return Asn1Value(tag, infra::Head(range, length));
    }

    Asn1Sequence::Asn1Sequence(infra::ConstByteRange range)
        : range(range)
    {}

    Asn1SequenceIterator Asn1Sequence::begin() const
    {
        return Asn1SequenceIterator(range);
    }

    Asn1SequenceIterator Asn1Sequence::end() const
    {
        return Asn1SequenceIterator();
    }

    Asn1Value Asn1Sequence::front() const
    {
        return *begin();
    }

    Asn1Value Asn1Sequence::operator[](std::size_t index) const
    {
        return *std::next(begin(), index);
    }

    bool Asn1Sequence::operator==(const Asn1Sequence& other) const
    {
        return range == other.range;
    }

    bool Asn1Sequence::operator!=(const Asn1Sequence& other) const
    {
        return !(*this == other);
    }
}
