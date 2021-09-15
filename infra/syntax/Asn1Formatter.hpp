#ifndef INFRA_ASN1_FORMATTER_HPP
#define INFRA_ASN1_FORMATTER_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/stream/SavedMarkerStream.hpp"
#include "infra/util/WithStorage.hpp"

namespace infra
{
    class Asn1ContainerFormatter;

    class Asn1Formatter
    {
    public:
        explicit Asn1Formatter(infra::DataOutputStream& stream);

        void Add(uint8_t value);
        void Add(uint32_t value);
        void Add(int32_t value);

        void AddSerial(infra::ConstByteRange serial);
        void AddBigNumber(infra::ConstByteRange number);
        void AddContextSpecific(uint8_t context, infra::ConstByteRange data);
        void AddObjectId(infra::ConstByteRange oid);
        void AddBitString(infra::ConstByteRange string);
        void AddPrintableString(infra::ConstByteRange string);
        void AddUtcTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
        void AddGeneralizedTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
        void AddTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);

        template<typename T>
        void AddOptional(infra::Optional<T> value);

        Asn1ContainerFormatter StartSequence();
        Asn1ContainerFormatter StartSet();
        Asn1ContainerFormatter StartContextSpecific(uint8_t context = 0);
        Asn1ContainerFormatter StartBitString();

        bool Failed() const;

    private:
        enum Tag : uint8_t
        {
            Integer = 0x02,
            BitString = 0x03,
            Null = 0x05,
            Oid = 0x06,
            Sequence = 0x10,
            Set = 0x11,
            PrintableString = 0x13,
            UtcTime = 0x17,
            GeneralizedTime = 0x18,
            Constructed = 0x20,
            ContextSpecific = 0x80
        };

    private:
        template<typename T>
        void AddTagLengthValue(uint8_t tag, uint32_t length, T value);
        void AddTagLength(uint8_t tag, uint32_t length);

    protected:
        infra::DataOutputStream stream;
    };

    template<typename T>
    void Asn1Formatter::AddOptional(infra::Optional<T> value)
    {
        if (value)
            Add(*value);
        else
            AddTagLength(Tag::Null, 0);
    }

    template<typename T>
    void Asn1Formatter::AddTagLengthValue(uint8_t tag, uint32_t length, T value)
    {
        AddTagLength(tag, length);
        stream << value;
    }

    class Asn1ContainerFormatter
        : public Asn1Formatter
    {
    public:
        Asn1ContainerFormatter(infra::DataOutputStream& stream, std::size_t sizeMarker);
        Asn1ContainerFormatter(const Asn1ContainerFormatter& other) = delete;
        Asn1ContainerFormatter(Asn1ContainerFormatter&& other);
        Asn1ContainerFormatter& operator=(const Asn1ContainerFormatter& other) = delete;
        Asn1ContainerFormatter& operator=(Asn1ContainerFormatter&& other);
        ~Asn1ContainerFormatter();

    private:
        std::size_t sizeMarker;
    };
}

#endif
