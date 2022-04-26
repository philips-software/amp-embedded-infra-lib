#ifndef INFRA_OUTPUT_STREAM_HPP
#define INFRA_OUTPUT_STREAM_HPP

#include "infra/stream/StreamErrorPolicy.hpp"
#include "infra/stream/StreamManipulators.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/IntegerNormalization.hpp"
#include "infra/util/Optional.hpp"
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
        StreamWriterDummy();

        virtual void Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy);
        virtual std::size_t Available() const;
    };

    class OutputStream
    {
    public:
        OutputStream(StreamWriter& writer, StreamErrorPolicy& errorPolicy);

    protected:
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

    class DataOutputStream
        : public OutputStream
    {
    public:
        template<class Writer>
        class WithWriter;
        class WithErrorPolicy;

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
        class WithWriter;
        class WithErrorPolicy;

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

            virtual void Stream(TextOutputStream& stream) override;

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

    template<class TheWriter>
    class DataOutputStream::WithWriter
        : private detail::StorageHolder<TheWriter, WithWriter<TheWriter>>
        , public DataOutputStream
    {
    public:
        template<class... Args>
        WithWriter(Args&&... args);
        template<class Storage, class... Args>
        WithWriter(Storage&& storage, SoftFail, Args&&... args);
        template<class Storage, class... Args>
        WithWriter(Storage&& storage, NoFail, Args&&... args);
        WithWriter(const WithWriter& other);
        WithWriter& operator=(const WithWriter& other) = delete;
        ~WithWriter() = default;

        TheWriter& Writer();

    private:
        StreamErrorPolicy errorPolicy;
    };

    class DataOutputStream::WithErrorPolicy
        : public DataOutputStream
    {
    public:
        WithErrorPolicy(StreamWriter& writer);
        WithErrorPolicy(StreamWriter& writer, SoftFail);
        WithErrorPolicy(StreamWriter& writer, NoFail);
        WithErrorPolicy(const WithErrorPolicy& other);
        ~WithErrorPolicy() = default;

    private:
        StreamErrorPolicy errorPolicy;
    };

    template<class TheWriter>
    class TextOutputStream::WithWriter
        : private detail::StorageHolder<TheWriter, WithWriter<TheWriter>>
        , public TextOutputStream
    {
    public:
        template<class... Args>
        WithWriter(Args&&... args);
        template<class Storage, class... Args>
        WithWriter(Storage&& storage, SoftFail, Args&&... args);
        template<class Storage, class... Args>
        WithWriter(Storage&& storage, NoFail, Args&&... args);
        WithWriter(const WithWriter& other);
        WithWriter& operator=(const WithWriter& other) = delete;
        ~WithWriter() = default;

        TheWriter& Writer();

    private:
        StreamErrorPolicy errorPolicy;
    };

    class TextOutputStream::WithErrorPolicy
        : public TextOutputStream
    {
    public:
        WithErrorPolicy(StreamWriter& writer);
        WithErrorPolicy(StreamWriter& writer, SoftFail);
        WithErrorPolicy(StreamWriter& writer, NoFail);
        WithErrorPolicy(const WithErrorPolicy& other);
        ~WithErrorPolicy() = default;

    private:
        StreamErrorPolicy errorPolicy;
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

    class AsBase64Helper
    {
    public:
        explicit AsBase64Helper(infra::ConstByteRange data);

        friend infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const AsBase64Helper& asBase64Helper);
        friend infra::TextOutputStream& operator<<(infra::TextOutputStream&& stream, const AsBase64Helper& asBase64Helper);

    private:
        infra::ConstByteRange data;
    };

    AsAsciiHelper AsAscii(infra::ConstByteRange data);
    AsHexHelper AsHex(infra::ConstByteRange data);
    AsBase64Helper AsBase64(infra::ConstByteRange data);

    template<class T>
    class ReservedProxy
    {
    public:
        explicit ReservedProxy(ByteRange range);

        ReservedProxy& operator=(const T& data);

    private:
        ByteRange range;
    };

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

    template<class TheWriter>
    template<class... Args>
    DataOutputStream::WithWriter<TheWriter>::WithWriter(Args&&... args)
        : detail::StorageHolder<TheWriter, WithWriter<TheWriter>>(std::forward<Args>(args)...)
        , DataOutputStream(this->storage, errorPolicy)
    {}

    template<class TheWriter>
    template<class Storage, class... Args>
    DataOutputStream::WithWriter<TheWriter>::WithWriter(Storage&& storage, SoftFail, Args&&... args)
        : detail::StorageHolder<TheWriter, WithWriter<TheWriter>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , DataOutputStream(this->storage, errorPolicy)
        , errorPolicy(softFail)
    {}

    template<class TheWriter>
    template<class Storage, class... Args>
    DataOutputStream::WithWriter<TheWriter>::WithWriter(Storage&& storage, NoFail, Args&&... args)
        : detail::StorageHolder<TheWriter, WithWriter<TheWriter>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , DataOutputStream(this->storage, errorPolicy)
        , errorPolicy(noFail)
    {}

    template<class TheWriter>
    DataOutputStream::WithWriter<TheWriter>::WithWriter(const WithWriter& other)
        : detail::StorageHolder<TheWriter, WithWriter<TheWriter>>(static_cast<detail::StorageHolder<TheWriter, WithWriter<TheWriter>>&>(other))
        , DataOutputStream(this->storage, errorPolicy)
        , errorPolicy(other.ErrorPolicy())
    {}

    template<class TheWriter>
    TheWriter& DataOutputStream::WithWriter<TheWriter>::Writer()
    {
        return this->storage;
    }

    template<class TheWriter>
    template<class... Args>
    TextOutputStream::WithWriter<TheWriter>::WithWriter(Args&&... args)
        : detail::StorageHolder<TheWriter, WithWriter<TheWriter>>(std::forward<Args>(args)...)
        , TextOutputStream(this->storage, errorPolicy)
    {}

    template<class TheWriter>
    template<class Storage, class... Args>
    TextOutputStream::WithWriter<TheWriter>::WithWriter(Storage&& storage, SoftFail, Args&&... args)
        : detail::StorageHolder<TheWriter, WithWriter<TheWriter>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , TextOutputStream(this->storage, errorPolicy)
        , errorPolicy(softFail)
    {}

    template<class TheWriter>
    template<class Storage, class... Args>
    TextOutputStream::WithWriter<TheWriter>::WithWriter(Storage&& storage, NoFail, Args&&... args)
        : detail::StorageHolder<TheWriter, WithWriter<TheWriter>>(std::forward<Storage>(storage), std::forward<Args>(args)...)
        , TextOutputStream(this->storage, errorPolicy)
        , errorPolicy(noFail)
    {}

    template<class TheWriter>
    TextOutputStream::WithWriter<TheWriter>::WithWriter(const WithWriter& other)
        : detail::StorageHolder<TheWriter, WithWriter<TheWriter>>(static_cast<detail::StorageHolder<TheWriter, WithWriter<TheWriter>>&>(other))
        , TextOutputStream(this->storage, errorPolicy)
        , errorPolicy(other.ErrorPolicy())
    {}

    template<class TheWriter>
    TheWriter& TextOutputStream::WithWriter<TheWriter>::Writer()
    {
        return this->storage;
    }

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
}

#endif
