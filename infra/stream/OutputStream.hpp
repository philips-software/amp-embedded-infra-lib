#ifndef INFRA_OUTPUT_STREAM_HPP
#define INFRA_OUTPUT_STREAM_HPP

#include "infra/stream/StreamErrorPolicy.hpp"
#include "infra/stream/StreamManipulators.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/IntegerNormalization.hpp"
#include <type_traits>

namespace infra
{
    class TextOutputStream;

    class StreamWriter
    {
    protected:
        StreamWriter() = default;
        StreamWriter(const StreamWriter&) = delete;
        StreamWriter& operator=(const StreamWriter&) = delete;
        ~StreamWriter() = default;

    public:
        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) = 0;
        virtual std::size_t Available() const = 0;

        bool Empty() const
        {
            return Available() == 0;
        }

        virtual std::size_t ConstructSaveMarker() const;
        virtual std::size_t GetProcessedBytesSince(std::size_t marker) const;
        virtual infra::ByteRange SaveState(std::size_t marker);
        virtual void RestoreState(infra::ByteRange range);
        virtual infra::ByteRange Overwrite(std::size_t marker);
    };

    class StreamWriterDummy
        : public StreamWriter
    {
    public:
        void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy) override;
        std::size_t Available() const override;
    };

    class OutputStream
    {
    public:
        OutputStream(StreamWriter& writer, StreamErrorPolicy& errorPolicy);

    protected:
#if defined(_MSC_VER) && _MSC_VER <= 1929
        /*
            Visual Studio 2019 version 16.11.2 and lower have an issue
            with some specific optimizations and OutputStream being
            trivially copyable. Resulting in unitialized object
            instances after copying an OutputStream.

            By defining a copy and or move constructor the object
            becomes non-trivially-copyable and the faulty optimization is not used.

            _MSC_VER 1929 is MSVC++ 14.28 that ships with Visual Studio 2019 version 16.11.2
        */
        OutputStream(const OutputStream& other);
        OutputStream(OutputStream&& other);

        OutputStream& operator=(const OutputStream&) = delete;
        OutputStream& operator=(OutputStream&&) = delete;
#endif
        ~OutputStream() = default;

    public:
        bool Failed() const;
        std::size_t SaveMarker() const;
        std::size_t ProcessedBytesSince(std::size_t marker) const;
        std::size_t Available() const;

        StreamWriter& Writer() const;
        StreamErrorPolicy& ErrorPolicy() const;

    private:
        StreamWriter& writer;
        StreamErrorPolicy& errorPolicy;
    };

    template<class Parent, class WriterType>
    class OutputStreamWithWriter //NOSONAR
        : private detail::StorageHolder<WriterType, OutputStreamWithWriter<Parent, WriterType>>
        , public Parent
    {
    public:
        OutputStreamWithWriter();
        template<class Arg>
        explicit OutputStreamWithWriter(Arg&& arg, std::enable_if_t<!std::is_same_v<OutputStreamWithWriter, std::remove_cv_t<std::remove_reference_t<Arg>>>, std::nullptr_t> = nullptr);
        template<class Arg0, class Arg1, class... Args>
        explicit OutputStreamWithWriter(Arg0&& arg0, Arg1&& arg1, Args&&... args);
        template<class Storage, class... Args>
        OutputStreamWithWriter(Storage&& storage, SoftFail, Args&&... args);
        template<class Storage, class... Args>
        OutputStreamWithWriter(Storage&& storage, NoFail, Args&&... args);
        OutputStreamWithWriter(const OutputStreamWithWriter& other);
        OutputStreamWithWriter& operator=(const OutputStreamWithWriter& other) = delete;
        ~OutputStreamWithWriter() = default;

        WriterType& Writer();

    private:
        StreamErrorPolicy errorPolicy;
    };

    template<class Parent>
    class OutputStreamWithErrorPolicy //NOSONAR
        : public Parent
    {
    public:
        explicit OutputStreamWithErrorPolicy(StreamWriter& writer);
        OutputStreamWithErrorPolicy(StreamWriter& writer, SoftFail);
        OutputStreamWithErrorPolicy(StreamWriter& writer, NoFail);
        OutputStreamWithErrorPolicy(const OutputStreamWithErrorPolicy& other);
        OutputStreamWithErrorPolicy& operator=(const OutputStreamWithErrorPolicy& other) = delete;
        ~OutputStreamWithErrorPolicy() = default;

    private:
        StreamErrorPolicy errorPolicy;
    };

    class DataOutputStream
        : public OutputStream
    {
    public:
        template<class Writer>
        using WithWriter = OutputStreamWithWriter<DataOutputStream, Writer>;
        using WithErrorPolicy = OutputStreamWithErrorPolicy<DataOutputStream>;

        using OutputStream::OutputStream;

        TextOutputStream operator<<(Text);

        template<class Data>
        DataOutputStream& operator<<(const Data& data);
        template<class Data>
        DataOutputStream& operator<<(MemoryRange<Data> data);
    };

    class TextOutputStream
        : public OutputStream
    {
    public:
        template<class Writer>
        using WithWriter = OutputStreamWithWriter<TextOutputStream, Writer>;
        using WithErrorPolicy = OutputStreamWithErrorPolicy<TextOutputStream>;

        TextOutputStream(StreamWriter& writer, StreamErrorPolicy& errorPolicy);

        TextOutputStream operator<<(Hex);
        TextOutputStream operator<<(Bin);
        TextOutputStream operator<<(Width width);
        DataOutputStream operator<<(Data);
        TextOutputStream& operator<<(Endl);

        TextOutputStream& operator<<(const char* zeroTerminatedString);
        TextOutputStream& operator<<(BoundedConstString string);
        TextOutputStream& operator<<(const std::string& string);
        TextOutputStream& operator<<(char c);
        TextOutputStream& operator<<(int8_t v);
        TextOutputStream& operator<<(uint8_t v);
        TextOutputStream& operator<<(int16_t v);
        TextOutputStream& operator<<(uint16_t v);
        TextOutputStream& operator<<(int32_t v);
        TextOutputStream& operator<<(uint32_t v);
        TextOutputStream& operator<<(int64_t v);
        TextOutputStream& operator<<(uint64_t v);
        TextOutputStream& operator<<(float v);

        template<class T>
        struct IntegralOrEnum
        {
            static constexpr bool value = std::is_integral<T>::value || std::is_enum<T>::value;
        };

        template<class T, typename std::enable_if<IntegralOrEnum<T>::value, T>::type* = nullptr>
        TextOutputStream& operator<<(T v)
        {
            using type = typename infra::NormalizedIntegralType<T>::type;

            return *this << static_cast<type>(v);
        }

        template<class... Args>
        void Format(const char* format, Args&&... arguments);

    private:
        class FormatterBase
        {
        public:
            FormatterBase() = default;

        protected:
            FormatterBase(const FormatterBase& other) = default;
            FormatterBase& operator=(const FormatterBase& other) = default;
            ~FormatterBase() = default;

        public:
            virtual void Stream(TextOutputStream& stream) = 0;
        };

        template<class T>
        class Formatter
            : public FormatterBase
        {
        public:
            explicit Formatter(T value);
            Formatter(const Formatter& other) = default;
            Formatter& operator=(const Formatter& other) = default;

            void Stream(TextOutputStream& stream) override;

        private:
            T value;
        };

        template<class T>
        Formatter<T> MakeFormatter(T&& argument);

    private:
        void OutputAsDecimal(uint64_t v, bool negative);
        void OutputAsBinary(uint64_t v, bool negative);
        void OutputAsHexadecimal(uint64_t v, bool negative);

        template<class... Formatters>
        void FormatHelper(const char* format, Formatters&&... formatters);
        void FormatArgs(const char* format, infra::MemoryRange<FormatterBase*> formatters);
        void OutputOptionalPadding(size_t paddingSize);

    private:
        enum class Radix
        {
            dec,
            bin,
            hex
        };

        Radix radix{ Radix::dec };
        Width width{ 0 };
    };

    class AsAsciiHelper
    {
    public:
        explicit AsAsciiHelper(infra::ConstByteRange data);

        friend infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const AsAsciiHelper& asAsciiHelper);

    private:
        infra::ConstByteRange data;
    };

    class AsHexHelper
    {
    public:
        explicit AsHexHelper(infra::ConstByteRange data);

        friend infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const AsHexHelper& asHexHelper);
        friend infra::TextOutputStream& operator<<(TextOutputStream&& stream, const AsHexHelper& asHexHelper);

    private:
        infra::ConstByteRange data;
    };

    class Base64Encoder
    {
    public:
        Base64Encoder(infra::TextOutputStream& stream);
        ~Base64Encoder();

        void Encode(infra::ConstByteRange data);

    private:
        infra::TextOutputStream& stream;
        uint8_t bitIndex = 2;
        uint8_t encodedByte = 0;
        uint32_t size = 0;
    };

    class AsBase64Helper
    {
    public:
        explicit AsBase64Helper(infra::ConstByteRange data);

        friend infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const AsBase64Helper& asBase64Helper);
        friend infra::TextOutputStream& operator<<(infra::TextOutputStream&& stream, const AsBase64Helper& asBase64Helper);

    private:
        infra::ConstByteRange data;
    };

    class AsCombinedBase64Helper
    {
    public:
        explicit AsCombinedBase64Helper(std::initializer_list<infra::ConstByteRange> ranges);

        friend infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const AsCombinedBase64Helper& asBase64Helper);
        friend infra::TextOutputStream& operator<<(infra::TextOutputStream&& stream, const AsCombinedBase64Helper& asBase64Helper);

    private:
        std::initializer_list<infra::ConstByteRange> ranges;
    };

    AsAsciiHelper AsAscii(infra::ConstByteRange data);
    AsHexHelper AsHex(infra::ConstByteRange data);
    AsBase64Helper AsBase64(infra::ConstByteRange data);
    AsCombinedBase64Helper AsBase64(std::initializer_list<infra::ConstByteRange> ranges);

    template<class T>
    class ReservedProxy
    {
    public:
        explicit ReservedProxy(ByteRange range);

        ReservedProxy& operator=(const T& data);

    private:
        ByteRange range;
    };

    template<class T>
    class JoinHelper;

    template<class T>
    TextOutputStream& operator<<(TextOutputStream& stream, const JoinHelper<T>& joinHelper);

    template<class T>
    class JoinHelper
    {
    public:
        using FunctionType = Function<void(TextOutputStream&, const T&)>;

        JoinHelper(BoundedConstString string, MemoryRange<T> range, const FunctionType& conversionFunction);

        friend TextOutputStream& operator<< <>(TextOutputStream& stream, const JoinHelper<T>& joinHelper);

    private:
        BoundedConstString string;
        MemoryRange<T> range;
        FunctionType conversionFunction;
    };

    template<class T>
    JoinHelper<T> Join(BoundedConstString string, MemoryRange<T> range, const typename JoinHelper<T>::FunctionType& func);

    template<class T>
    JoinHelper<T> Join(BoundedConstString string, MemoryRange<T> range);

    ////    Implementation    ////

    template<class Data>
    DataOutputStream& DataOutputStream::operator<<(const Data& data)
    {
        ConstByteRange dataRange(ReinterpretCastByteRange(MakeRange(&data, &data + 1)));
        Writer().Insert(dataRange, ErrorPolicy());
        return *this;
    }

    template<class Data>
    DataOutputStream& DataOutputStream::operator<<(MemoryRange<Data> data)
    {
        ConstByteRange dataRange(ReinterpretCastByteRange(data));
        Writer().Insert(dataRange, ErrorPolicy());
        return *this;
    }

    template<class Parent, class WriterType>
    OutputStreamWithWriter<Parent, WriterType>::OutputStreamWithWriter()
        : Parent(this->storage, errorPolicy)
    {}

    template<class Parent, class WriterType>
    template<class Arg>
    OutputStreamWithWriter<Parent, WriterType>::OutputStreamWithWriter(Arg&& arg, std::enable_if_t<!std::is_same_v<OutputStreamWithWriter, std::remove_cv_t<std::remove_reference_t<Arg>>>, std::nullptr_t>)
        : detail::StorageHolder<WriterType, OutputStreamWithWriter<Parent, WriterType>>(std::forward<Arg>(arg))
        , Parent(this->storage, errorPolicy)
    {}

    template<class Parent, class WriterType>
    template<class Arg0, class Arg1, class... Args>
    OutputStreamWithWriter<Parent, WriterType>::OutputStreamWithWriter(Arg0&& arg0, Arg1&& arg1, Args&&... args)
        : detail::StorageHolder<WriterType, OutputStreamWithWriter<Parent, WriterType>>(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Args>(args)...)
        , Parent(this->storage, errorPolicy)
    {}

    template<class Parent, class WriterType>
    template<class Storage, class... Args>
    OutputStreamWithWriter<Parent, WriterType>::OutputStreamWithWriter(Storage&& storage, SoftFail, Args&&... args)
        : detail::StorageHolder<WriterType, OutputStreamWithWriter<Parent, WriterType>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , Parent(this->storage, errorPolicy)
        , errorPolicy(softFail)
    {}

    template<class Parent, class WriterType>
    template<class Storage, class... Args>
    OutputStreamWithWriter<Parent, WriterType>::OutputStreamWithWriter(Storage&& storage, NoFail, Args&&... args)
        : detail::StorageHolder<WriterType, OutputStreamWithWriter<Parent, WriterType>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , Parent(this->storage, errorPolicy)
        , errorPolicy(noFail)
    {}

    template<class Parent, class WriterType>
    OutputStreamWithWriter<Parent, WriterType>::OutputStreamWithWriter(const OutputStreamWithWriter& other)
        : detail::StorageHolder<WriterType, OutputStreamWithWriter<Parent, WriterType>>(static_cast<detail::StorageHolder<WriterType, OutputStreamWithWriter<Parent, WriterType>>&>(other))
        , Parent(this->storage, errorPolicy)
        , errorPolicy(other.ErrorPolicy())
    {}

    template<class Parent, class WriterType>
    WriterType& OutputStreamWithWriter<Parent, WriterType>::Writer()
    {
        return this->storage;
    }

    template<class Parent>
    OutputStreamWithErrorPolicy<Parent>::OutputStreamWithErrorPolicy(StreamWriter& writer)
        : Parent(writer, errorPolicy)
    {}

    template<class Parent>
    OutputStreamWithErrorPolicy<Parent>::OutputStreamWithErrorPolicy(StreamWriter& writer, SoftFail)
        : Parent(writer, errorPolicy)
        , errorPolicy(softFail)
    {}

    template<class Parent>
    OutputStreamWithErrorPolicy<Parent>::OutputStreamWithErrorPolicy(StreamWriter& writer, NoFail)
        : Parent(writer, errorPolicy)
        , errorPolicy(noFail)
    {}

    template<class Parent>
    OutputStreamWithErrorPolicy<Parent>::OutputStreamWithErrorPolicy(const OutputStreamWithErrorPolicy& other)
        : Parent(other.Writer(), errorPolicy)
        , errorPolicy(other.ErrorPolicy())
    {}

    template<class T>
    TextOutputStream::Formatter<T>::Formatter(T value)
        : value(value)
    {}

    template<class T>
    void TextOutputStream::Formatter<T>::Stream(TextOutputStream& stream)
    {
        stream << value;
    }

    template<class T>
    TextOutputStream::Formatter<T> TextOutputStream::MakeFormatter(T&& argument)
    {
        return Formatter<T>(std::forward<T>(argument));
    }

    template<class... Args>
    void TextOutputStream::Format(const char* format, Args&&... arguments)
    {
        FormatHelper(format, MakeFormatter(arguments)...);
    }

    template<class... Args>
    void TextOutputStream::FormatHelper(const char* format, Args&&... arguments)
    {
        std::array<FormatterBase*, sizeof...(Args)> formatters = { &arguments... };
        FormatArgs(format, formatters);
    }

    template<class T>
    ReservedProxy<T>::ReservedProxy(ByteRange range)
        : range(range)
    {}

    template<class T>
    ReservedProxy<T>& ReservedProxy<T>::operator=(const T& data)
    {
        if (range.size() == sizeof(data))
            Copy(infra::MakeByteRange(data), range);

        return *this;
    }

    template<class T>
    JoinHelper<T>::JoinHelper(BoundedConstString string, infra::MemoryRange<T> range, const FunctionType& conversionFunction)
        : string{ string }
        , range{ range }
        , conversionFunction{ conversionFunction }
    {}

    template<class T>
    TextOutputStream& operator<<(TextOutputStream& stream, const JoinHelper<T>& joinHelper)
    {
        if (!joinHelper.range.empty())
        {
            for (const auto& obj : DiscardTail(joinHelper.range, 1))
            {
                joinHelper.conversionFunction(stream, obj);
                stream << joinHelper.string;
            }

            joinHelper.conversionFunction(stream, joinHelper.range.back());
        }

        return stream;
    }

    template<class T>
    JoinHelper<T> Join(BoundedConstString string, MemoryRange<T> range, const typename JoinHelper<T>::FunctionType& conversionFunction)
    {
        return { string, range, conversionFunction };
    }

    template<class T>
    JoinHelper<T> Join(BoundedConstString string, MemoryRange<T> range)
    {
        return { string, range, [](TextOutputStream& stream, const T& obj)
            {
                stream << obj;
            } };
    }
}

#endif
