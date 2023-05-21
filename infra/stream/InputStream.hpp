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

    template<class Parent, class ReaderType>
    class InputStreamWithReader
        : private detail::StorageHolder<ReaderType, InputStreamWithReader<Parent, ReaderType>>
        , public Parent
    {
    public:
        InputStreamWithReader();
        template<class Arg>
        explicit InputStreamWithReader(Arg&& arg, std::enable_if_t<!std::is_same_v<InputStreamWithReader, std::remove_cv_t<std::remove_reference_t<Arg>>>, std::nullptr_t> = nullptr);
        template<class Arg0, class Arg1, class... Args>
        explicit InputStreamWithReader(Arg0&& arg0, Arg1&& arg1, Args&&... args);
        template<class Storage, class... Args>
        InputStreamWithReader(Storage&& storage, const SoftFail&, Args&&... args);
        template<class Storage, class... Args>
        InputStreamWithReader(Storage&& storage, const NoFail&, Args&&... args);
        InputStreamWithReader(const InputStreamWithReader& other); //NOSONAR
        InputStreamWithReader& operator=(const InputStreamWithReader& other) = delete;
        ~InputStreamWithReader() = default;

        ReaderType& Reader();

    private:
        StreamErrorPolicy errorPolicy;
    };

    template<class Parent>
    class InputStreamWithErrorPolicy
        : public Parent
    {
    public:
        explicit InputStreamWithErrorPolicy(StreamReader& reader);
        InputStreamWithErrorPolicy(StreamReader& reader, SoftFail);
        InputStreamWithErrorPolicy(StreamReader& reader, NoFail);
        InputStreamWithErrorPolicy(const InputStreamWithErrorPolicy& other); //NOSONAR
        InputStreamWithErrorPolicy& operator=(const InputStreamWithErrorPolicy& other) = delete;
        ~InputStreamWithErrorPolicy() = default;

    private:
        StreamErrorPolicy errorPolicy;
    };

    class DataInputStream
        : public InputStream
    {
    public:
        template<class Reader>
        using WithReader = InputStreamWithReader<DataInputStream, Reader>;
        using WithErrorPolicy = InputStreamWithErrorPolicy<DataInputStream>;

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
        using WithReader = InputStreamWithReader<TextInputStream, Reader>;
        using WithErrorPolicy = InputStreamWithErrorPolicy<TextInputStream>;

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

    std::size_t Base64DecodedSize(infra::BoundedConstString encodedContents);

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

    template<class Parent, class ReaderType>
    InputStreamWithReader<Parent, ReaderType>::InputStreamWithReader()
        : Parent(this->storage, errorPolicy)
    {}

    template<class Parent, class ReaderType>
    template<class Arg>
    InputStreamWithReader<Parent, ReaderType>::InputStreamWithReader(Arg&& arg, std::enable_if_t<!std::is_same_v<InputStreamWithReader, std::remove_cv_t<std::remove_reference_t<Arg>>>, std::nullptr_t>)
        : detail::StorageHolder<ReaderType, InputStreamWithReader<Parent, ReaderType>>(std::forward<Arg>(arg))
        , Parent(this->storage, errorPolicy)
    {}

    template<class Parent, class ReaderType>
    template<class Arg0, class Arg1, class... Args>
    InputStreamWithReader<Parent, ReaderType>::InputStreamWithReader(Arg0&& arg0, Arg1&& arg1, Args&&... args)
        : detail::StorageHolder<ReaderType, InputStreamWithReader<Parent, ReaderType>>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Args>(args)...)
        , Parent(this->storage, errorPolicy)
    {}

    template<class Parent, class ReaderType>
    template<class Storage, class... Args>
    InputStreamWithReader<Parent, ReaderType>::InputStreamWithReader(Storage&& storage, const SoftFail&, Args&&... args)
        : detail::StorageHolder<ReaderType, InputStreamWithReader<Parent, ReaderType>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , Parent(this->storage, errorPolicy)
        , errorPolicy(softFail)
    {}

    template<class Parent, class ReaderType>
    template<class Storage, class... Args>
    InputStreamWithReader<Parent, ReaderType>::InputStreamWithReader(Storage&& storage, const NoFail&, Args&&... args)
        : detail::StorageHolder<ReaderType, InputStreamWithReader<Parent, ReaderType>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , Parent(this->storage, errorPolicy)
        , errorPolicy(noFail)
    {}

    template<class Parent, class ReaderType>
    InputStreamWithReader<Parent, ReaderType>::InputStreamWithReader(const InputStreamWithReader& other)
        : detail::StorageHolder<ReaderType, InputStreamWithReader<Parent, ReaderType>>(static_cast<const detail::StorageHolder<ReaderType, InputStreamWithReader<Parent, ReaderType>>&>(other))
        , Parent(this->storage, errorPolicy)
        , errorPolicy(other.ErrorPolicy())
    {}

    template<class Parent, class ReaderType>
    ReaderType& InputStreamWithReader<Parent, ReaderType>::InputStreamWithReader::Reader()
    {
        return this->storage;
    }

    template<class Parent>
    InputStreamWithErrorPolicy<Parent>::InputStreamWithErrorPolicy(StreamReader& reader)
        : Parent(reader, errorPolicy)
    {}

    template<class Parent>
    InputStreamWithErrorPolicy<Parent>::InputStreamWithErrorPolicy(StreamReader& reader, SoftFail)
        : Parent(reader, errorPolicy)
        , errorPolicy(softFail)
    {}

    template<class Parent>
    InputStreamWithErrorPolicy<Parent>::InputStreamWithErrorPolicy(StreamReader& reader, NoFail)
        : Parent(reader, errorPolicy)
        , errorPolicy(noFail)
    {}

    template<class Parent>
    InputStreamWithErrorPolicy<Parent>::InputStreamWithErrorPolicy(const InputStreamWithErrorPolicy& other)
        : Parent(other.Reader(), errorPolicy)
        , errorPolicy(other.ErrorPolicy())
    {}
}

#endif
