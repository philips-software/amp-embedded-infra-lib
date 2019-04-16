#ifndef INFRA_FORMATTER_HPP
#define INFRA_FORMATTER_HPP

#include "infra/stream/StringOutputStream.hpp"

namespace infra
{
    enum class FormatAlign
    {
        nothing,
        left,
        right,
        center
    };

    struct FormatSpec
    {
        explicit FormatSpec(const char*& format);

        std::size_t width{ 0 };
        char type{ '\0' };
        char fill{ ' ' };
        FormatAlign align{ FormatAlign::nothing };

    private:
        void ParseAlign();
        void ParseZero();
        void ParseWidth();
        void ParseType();

    private:
        const char*& format;
    };

    class FormatterBase
    {
    public:
        virtual ~FormatterBase() = default;
        virtual void Format(TextOutputStream& stream, FormatSpec& spec) = 0;

    protected:
        void RawFormat(TextOutputStream& stream, const char* text, std::size_t length, const FormatSpec& spec) const;
        void SignedInteger(int64_t value, TextOutputStream& stream, FormatSpec& spec) const;
        void UnsignedInteger(uint64_t value, TextOutputStream& stream, FormatSpec& spec) const;

    private:
        void RawInteger(uint64_t value, bool negative, uint8_t dotPos, TextOutputStream& stream, FormatSpec& spec) const;
    };

    template<typename T>
    class Formatter final
        : public FormatterBase
    {
    public:
        explicit Formatter(const T& value);
        explicit Formatter(typename std::remove_reference<T>::type&& value);

        void Format(TextOutputStream& stream, FormatSpec& spec) override;

    private:
        T value;
    };

    template<typename T>
    Formatter<T>::Formatter(const T& value)
        : value(value)
    {}

    template<typename T>
    Formatter<T>::Formatter(typename std::remove_reference<T>::type&& value)
        : value(std::move(value))
    {}

    class FormatWorker
    {
    public:
        explicit FormatWorker(TextOutputStream& writer, const char* formatStr, std::vector<FormatterBase*>& formatters);

    private:
        bool IsEndFormat() const;
        uint32_t ParseIndex();

    private:
        const char* format;
        uint32_t autoIndex{};
    };

    template<class T>
    struct DecayArray
    {
        using type = typename std::conditional <
            std::is_integral<typename std::remove_reference<T>::type>::value,
            typename std::remove_reference<T>::type,
            T>::type;
    };

    template<class T, std::size_t N>
    struct DecayArray<T(&)[N]>
    {
        using type = T*;
    };

    template<class T>
    Formatter<typename DecayArray<T>::type> MakeFormatter(T&& v)
    {
        return Formatter<typename DecayArray<T>::type>(std::forward<T>(v));
    }

    template<>
    void Formatter<int64_t>::Format(TextOutputStream& stream, FormatSpec& spec);

    template<>
    void Formatter<int32_t>::Format(TextOutputStream& stream, FormatSpec& spec);

    template<>
    void Formatter<int16_t>::Format(TextOutputStream& stream, FormatSpec& spec);

    template<>
    void Formatter<int8_t>::Format(TextOutputStream& stream, FormatSpec& spec);

    template<>
    void Formatter<uint64_t>::Format(TextOutputStream& stream, FormatSpec& spec);

    template<>
    void Formatter<uint32_t>::Format(TextOutputStream& stream, FormatSpec& spec);

    template<>
    void Formatter<uint16_t>::Format(TextOutputStream& stream, FormatSpec& spec);

    template<>
    void Formatter<uint8_t>::Format(TextOutputStream& stream, FormatSpec& spec);

    template<>
    void Formatter<bool>::Format(TextOutputStream& stream, FormatSpec& spec);

    template<>
    void Formatter<char const*>::Format(TextOutputStream& stream, FormatSpec& spec);

    template<class... Args>
    class FormatHelper
    {
    public:
        template<std::size_t... Is>
        std::vector<infra::FormatterBase*> Make(std::index_sequence<Is...>)
        {
            return{ &std::get<Is>(args)... };
        }

        std::vector<infra::FormatterBase*> MakeFormatter()
        {
            return Make(std::index_sequence_for<Args...>{});
        };

        explicit FormatHelper(const char* format, Args&&... args)
            : format(format)
            , args(std::forward<Args>(args)...)
            , formatters{ MakeFormatter() }
        {}


        friend TextOutputStream& operator<<(TextOutputStream& stream, FormatHelper&& f)
        {
            FormatWorker(stream, f.format, f.formatters);
            return stream;
        }

        friend TextOutputStream& operator<<(TextOutputStream& stream, FormatHelper& f)
        {
            FormatWorker(stream, f->format, f->formatters);
            return stream;
        }

    private:
        const char* format;
        std::tuple<Args ...> args;
        std::vector<infra::FormatterBase*> formatters{ sizeof...(Args), nullptr };
        uint32_t autoIndex{};
    };

    template<class... Args>
    auto Format(const char* format, Args&&... args)
    {
        return FormatHelper<Formatter<typename DecayArray<Args>::type>...>(format, MakeFormatter(std::forward<Args>(args))...);
    }
}
#endif
