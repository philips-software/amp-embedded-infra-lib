#ifndef INFRA_ASN1_HPP
#define INFRA_ASN1_HPP

#include "infra/util/ByteRange.hpp"
#include <utility>

namespace infra
{
    class Asn1Sequence;

    class Asn1Value
    {
    public:
        Asn1Value(uint8_t tag, infra::ConstByteRange range);

        Asn1Sequence Sequence() const;
        infra::ConstByteRange Integer() const;

    private:
        uint8_t tag;
        infra::ConstByteRange range;

        friend class Asn1SequenceIterator;
    };

    class Asn1SequenceIterator
        : public std::iterator<std::forward_iterator_tag, Asn1SequenceIterator>
    {
    public:
        Asn1SequenceIterator();
        explicit Asn1SequenceIterator(infra::ConstByteRange range);

        bool operator==(const Asn1SequenceIterator& other) const;
        bool operator!=(const Asn1SequenceIterator& other) const;

        Asn1Value& operator*();
        const Asn1Value& operator*() const;
        Asn1Value* operator->();
        const Asn1Value* operator->() const;

        Asn1SequenceIterator& operator++();
        Asn1SequenceIterator operator++(int);

    private:
        Asn1Value MakeValue() const;

    private:
        infra::ConstByteRange range;
        Asn1Value currentValue;
    };

    class Asn1Sequence
    {
    public:
        explicit Asn1Sequence(infra::ConstByteRange range);

        Asn1SequenceIterator begin() const;
        Asn1SequenceIterator end() const;

        Asn1Value front() const;
        Asn1Value operator[](std::size_t index) const;

        bool operator==(const Asn1Sequence& other) const;
        bool operator!=(const Asn1Sequence& other) const;

    private:
        infra::ConstByteRange range;
    };
}

#endif
