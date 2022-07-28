#ifndef INFRA_XML_FORMATTER_HPP
#define INFRA_XML_FORMATTER_HPP

#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/WithStorage.hpp"

namespace infra
{
    class XmlTagFormatter;

    class XmlFormatter
    {
    public:
        using WithStringStream = infra::WithStorage<XmlFormatter, infra::StringOutputStream>;

        explicit XmlFormatter(infra::TextOutputStream& stream);
        XmlFormatter(const XmlFormatter& other) = delete;
        XmlFormatter& operator=(const XmlFormatter& other) = delete;
        ~XmlFormatter() = default;

        XmlTagFormatter Tag(const char* tagName);

    private:
        infra::Optional<infra::TextOutputStream::WithErrorPolicy> stream;
    };

    class XmlTagFormatter
    {
    public:
        XmlTagFormatter(infra::TextOutputStream& stream, const char* tagName);
        XmlTagFormatter(const XmlTagFormatter& other) = delete;
        XmlTagFormatter(XmlTagFormatter&& other) noexcept;
        XmlTagFormatter& operator=(const XmlTagFormatter& other) = delete;
        XmlTagFormatter& operator=(XmlTagFormatter&& other) noexcept;
        ~XmlTagFormatter();

        void Attribute(const char* name, infra::BoundedConstString value);
        void Content(infra::BoundedConstString string);
        XmlTagFormatter Tag(const char* tagName);
        void Element(const char* tagName, infra::BoundedConstString content);

    private:
        void CloseBeginTag();

    private:
        infra::Optional<infra::TextOutputStream::WithErrorPolicy> stream;
        const char* tagName = nullptr;
        bool empty = true;
    };
}

#endif
