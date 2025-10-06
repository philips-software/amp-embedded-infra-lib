#include "infra/syntax/XmlFormatter.hpp"
#include "infra/syntax/EscapeCharacterHelper.hpp"
#include "infra/util/Optional.hpp"
#include <utility>

namespace infra
{
    namespace
    {
        void ReplaceEscapeCharacter(infra::TextOutputStream& stream, char c)
        {
            switch (c)
            {
                case '<':
                    stream << "&lt;";
                    break;
                case '>':
                    stream << "&gt;";
                    break;
                case '&':
                    stream << "&amp;";
                    break;
                case '\'':
                    stream << "&apos;";
                    break;
                case '"':
                    stream << "&quot;";
                    break;
            }
        }

        void InsertEscapedContent(infra::TextOutputStream& stream, infra::BoundedConstString content)
        {
            infra::InsertEscapedContent(stream, content, "<>&'\"", ReplaceEscapeCharacter);
        }
    }

    XmlFormatter::XmlFormatter(infra::TextOutputStream& stream)
        : stream(std::in_place, stream.Writer(), infra::noFail)
    {
        *this->stream << R"(<?xml version="1.0" encoding="ISO-8859-1" ?>)";
    }

    XmlTagFormatter XmlFormatter::Tag(const char* tagName)
    {
        return XmlTagFormatter(*stream, tagName);
    }

    XmlTagFormatter::XmlTagFormatter(infra::TextOutputStream& stream, const char* tagName)
        : stream(std::in_place, stream.Writer(), infra::noFail)
        , tagName(tagName)
    {
        *this->stream << "<" << tagName;
    }

    XmlTagFormatter::~XmlTagFormatter()
    {
        if (stream.has_value())
            return;

        if (empty)
            *stream << " />";
        else
            *stream << "</" << tagName << ">";
    }

    XmlTagFormatter::XmlTagFormatter(XmlTagFormatter&& other) noexcept
        : stream(other.stream)
    {
        stream.emplace(*other.stream);
        other.stream.reset();
    }

    XmlTagFormatter& XmlTagFormatter::operator=(XmlTagFormatter&& other) noexcept
    {
        stream.emplace(*other.stream);
        other.stream.reset();

        return *this;
    }

    void XmlTagFormatter::Attribute(const char* name, infra::BoundedConstString value)
    {
        *stream << " " << name << R"(=")" << value << R"(")";
    }

    void XmlTagFormatter::Content(infra::BoundedConstString string)
    {
        CloseBeginTag();
        InsertEscapedContent(*stream, string);
    }

    XmlTagFormatter XmlTagFormatter::Tag(const char* tagName)
    {
        CloseBeginTag();
        return XmlTagFormatter(*stream, tagName);
    }

    void XmlTagFormatter::Element(const char* tagName, infra::BoundedConstString content)
    {
        auto element = Tag(tagName);
        element.Content(content);
    }

    void XmlTagFormatter::CloseBeginTag()
    {
        if (empty)
        {
            empty = false;
            *stream << ">";
        }
    }
}
