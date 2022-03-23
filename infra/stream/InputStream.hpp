#ifndef INFRA_INPUT_STREAM_HPP
#define INFRA_INPUT_STREAM_HPP

#include "infra/stream/StreamErrorPolicy.hpp"
#include "infra/stream/StreamManipulators.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Optional.hpp"
#include <cstdlib>
#include <type_traits>

namespace infra
{
    class TextInputStream;

    class StreamReader
    {
    protected:
        StreamReader() = default;
        StreamReader(const StreamReader& other) = delete;
        StreamReader& operator=(const StreamReader& other) = delete;
        ~StreamReader() = default;

    public:
        virtual void Extract(ByteRange range, StreamErrorPolicy& errorPolicy) = 0;
        virtual uint8_t Peek(StreamErrorPolicy& errorPolicy) = 0;
        virtual ConstByteRange ExtractContiguousRange(std::size_t max) = 0;
        virtual ConstByteRange PeekContiguousRange(std::size_t start) = 0;

        virtual bool Empty() const = 0;
        virtual std::size_t Available() const = 0;
    };

    class StreamReaderWithRewinding
        : public StreamReader
    {
    public:
        virtual std::size_t ConstructSaveMarker() const = 0;
        virtual void Rewind(std::size_t marker) = 0;
    };

    class InputStream
    {
    public:
        InputStream(StreamReader& reader, StreamErrorPolicy& errorPolicy);

    protected:
        ~InputStream() = default;

    public:
        bool Empty() const;
        std::size_t Available() const;
        ConstByteRange ContiguousRange(std::size_t max = std::numeric_limits<std::size_t>::max()) const;
        ConstByteRange PeekContiguousRange(std::size_t start = 0) const;
        void Consume(std::size_t amount);
        bool Failed() const;

        StreamReader& Reader() const;
        StreamErrorPolicy& ErrorPolicy() const;

    private:
        StreamReader& reader;
        StreamErrorPolicy& errorPolicy;
    };

    class DataInputStream
        : public InputStream
    {
    public:
        template<class Reader>
            class WithReader;
        class WithErrorPolicy;

        using InputStream::InputStream;

        TextInputStream operator>>(Text);
        DataInputStream& operator>>(ByteRange range);
        template<class Data>
            DataInputStream& operator>>(Data& data);

        template<class Data>
            Data Extract();
    };

    class TextInputStream
        : public InputStream
    {
    public:
        template<class Reader>
            class WithReader;
        class WithErrorPolicy;

        using InputStream::InputStream;

        DataInputStream operator>>(Data);
        TextInputStream operator>>(Hex);
        TextInputStream operator>>(Width width);

        TextInputStream& operator>>(MemoryRange<char> text);
        TextInputStream& operator>>(char& c);
        TextInputStream& operator>>(int8_t& v);
        TextInputStream& operator>>(int16_t& v);
        TextInputStream& operator>>(int32_t& v);
        TextInputStream& operator>>(int64_t& v);
        TextInputStream& operator>>(uint8_t& v);
        TextInputStream& operator>>(uint16_t& v);
        TextInputStream& operator>>(uint32_t& v);
        TextInputStream& operator>>(uint64_t& v);
        TextInputStream& operator>>(float& v);
        TextInputStream& operator>>(BoundedString& v);
        TextInputStream& operator>>(const char* literal);

    private:
        void SkipSpaces();
        void Read(int32_t& v);
        void Read(int64_t& v);
        void Read(uint32_t& v);
        void Read(uint64_t& v);
        void ReadAsDecimal(int32_t& v);
        void ReadAsDecimal(int64_t& v);
        void ReadAsDecimal(uint32_t& v);
        void ReadAsDecimal(uint64_t& v);
        void ReadAsHex(int32_t& v);
        void ReadAsHex(int64_t& v);
        void ReadAsHex(uint32_t& v);
        void ReadAsHex(uint64_t& v);

        bool isDecimal = true;
        infra::Optional<std::size_t> width;
    };

    template<class TheReader>
    class DataInputStream::WithReader
        : private detail::StorageHolder<TheReader, WithReader<TheReader>>
        , public DataInputStream
    {
    public:
        template<class... Args>
            WithReader(Args&&... args);
        template<class Storage, class... Args>
            WithReader(Storage&& storage, const SoftFail&, Args&&... args);
        template<class Storage, class... Args>
            WithReader(Storage&& storage, const NoFail&, Args&&... args);
        WithReader(const WithReader& other);
        WithReader& operator=(const WithReader& other) = delete;
        ~WithReader() = default;

        TheReader& Reader();

    private:
        StreamErrorPolicy errorPolicy;
    };

    class DataInputStream::WithErrorPolicy
        : public DataInputStream
    {
    public:
        WithErrorPolicy(StreamReader& reader);
        WithErrorPolicy(StreamReader& reader, SoftFail);
        WithErrorPolicy(StreamReader& reader, NoFail);
        WithErrorPolicy(const WithErrorPolicy& other);
        ~WithErrorPolicy() = default;

    private:
        StreamErrorPolicy errorPolicy;
    };

    template<class TheReader>
    class TextInputStream::WithReader
        : private detail::StorageHolder<TheReader, WithReader<TheReader>>
        , public TextInputStream
    {
    public:
        template<class... Args>
            WithReader(Args&&... args);
        template<class Storage, class... Args>
            WithReader(Storage&& storage, const SoftFail&, Args&&... args);
        template<class Storage, class... Args>
            WithReader(Storage&& storage, const NoFail&, Args&&... args);
        WithReader(const WithReader& other);
        WithReader& operator=(const WithReader& other) = delete;
        ~WithReader() = default;

        TheReader& Reader();

    private:
        StreamErrorPolicy errorPolicy;
    };

    class TextInputStream::WithErrorPolicy
        : public TextInputStream
    {
    public:
        WithErrorPolicy(StreamReader& writer);
        WithErrorPolicy(StreamReader& writer, SoftFail);
        WithErrorPolicy(StreamReader& writer, NoFail);
        WithErrorPolicy(const WithErrorPolicy& other);
        ~WithErrorPolicy() = default;

    private:
        StreamErrorPolicy errorPolicy;
    };

    class FromHexHelper
    {
    public:
        explicit FromHexHelper(infra::ByteRange data);

        friend infra::TextInputStream& operator>>(infra::TextInputStream& stream, FromHexHelper helper);
        friend infra::TextInputStream& operator>>(infra::TextInputStream&& stream, FromHexHelper helper);

    private:
        infra::ByteRange data;
    };

    class FromBase64Helper
    {
    public:
        explicit FromBase64Helper(infra::ByteRange data);

        friend infra::TextInputStream& operator>>(infra::TextInputStream& stream, FromBase64Helper helper);
        friend infra::TextInputStream& operator>>(infra::TextInputStream&& stream, FromBase64Helper helper);

    private:
        infra::ByteRange data;
    };

    FromHexHelper FromHex(infra::ByteRange data);
    FromBase64Helper FromBase64(infra::ByteRange data);

    size_t Base64DecodedSize(infra::BoundedConstString encodedContents);

    ////    Implementation    ////

    template<class Data>
    DataInputStream& DataInputStream::operator>>(Data& data)
    {
        MemoryRange<typename std::remove_const<uint8_t>::type> dataRange(ReinterpretCastMemoryRange<typename std::remove_const<uint8_t>::type>(MakeRange(&data, &data + 1)));
        Reader().Extract(dataRange, ErrorPolicy());
        return *this;
    }

    template<class Data>
    Data DataInputStream::Extract()
    {
        Data result{};
        *this >> result;
        return result;
    }

    template<class TheReader>
    template<class... Args>
    DataInputStream::WithReader<TheReader>::WithReader(Args&&... args)
        : detail::StorageHolder<TheReader, WithReader<TheReader>>(std::forward<Args>(args)...)
        , DataInputStream(this->storage, errorPolicy)
    {}

    template<class TheReader>
    template<class Storage, class... Args>
    DataInputStream::WithReader<TheReader>::WithReader(Storage&& storage, const SoftFail&, Args&&... args)
        : detail::StorageHolder<TheReader, WithReader<TheReader>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , DataInputStream(this->storage, errorPolicy)
        , errorPolicy(softFail)
    {}

    template<class TheReader>
    template<class Storage, class... Args>
    DataInputStream::WithReader<TheReader>::WithReader(Storage&& storage, const NoFail&, Args&&... args)
        : detail::StorageHolder<TheReader, WithReader<TheReader>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , DataInputStream(this->storage, errorPolicy)
        , errorPolicy(noFail)
    {}

    template<class TheReader>
    DataInputStream::WithReader<TheReader>::WithReader(const WithReader& other)
        : detail::StorageHolder<TheReader, WithReader<TheReader>>(static_cast<const detail::StorageHolder<TheReader, WithReader<TheReader>>&>(other))
        , DataInputStream(this->storage, errorPolicy)
        , errorPolicy(other.ErrorPolicy())
    {}

    template<class TheReader>
    TheReader& DataInputStream::WithReader<TheReader>::Reader()
    {
        return this->storage;
    }

    template<class TheReader>
    template<class... Args>
    TextInputStream::WithReader<TheReader>::WithReader(Args&&... args)
        : detail::StorageHolder<TheReader, WithReader<TheReader>>(std::forward<Args>(args)...)
        , TextInputStream(this->storage, errorPolicy)
    {}

    template<class TheReader>
    template<class Storage, class... Args>
    TextInputStream::WithReader<TheReader>::WithReader(Storage&& storage, const SoftFail&, Args&&... args)
        : detail::StorageHolder<TheReader, WithReader<TheReader>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , TextInputStream(this->storage, errorPolicy)
        , errorPolicy(softFail)
    {}

    template<class TheReader>
    template<class Storage, class... Args>
    TextInputStream::WithReader<TheReader>::WithReader(Storage&& storage, const NoFail&, Args&&... args)
        : detail::StorageHolder<TheReader, WithReader<TheReader>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , TextInputStream(this->storage, errorPolicy)
        , errorPolicy(noFail)
    {}

    template<class TheReader>
    TextInputStream::WithReader<TheReader>::WithReader(const WithReader& other)
        : detail::StorageHolder<TheReader, WithReader<TheReader>>(static_cast<detail::StorageHolder<TheReader, WithReader<TheReader>>&>(other))
        , TextInputStream(this->storage, errorPolicy)
        , errorPolicy(other.ErrorPolicy())
    {}

    template<class TheReader>
    TheReader& TextInputStream::WithReader<TheReader>::Reader()
    {
        return this->storage;
    }
}

#endif
