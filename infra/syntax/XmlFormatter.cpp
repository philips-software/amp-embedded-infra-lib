#include "infra/syntax/XmlFormatter.hpp"

namespace
{
    namespace
    {
        void InsertPredefinedEntity(infra::TextOutputStream& stream, char c)
        {
            switch (c)
            {
            case '<':  stream << "&lt;"; break;
            case '>':  stream << "&gt;"; break;
            case '&':  stream << "&amp;"; break;
            case '\'': stream << "&apos;"; break;
            case '"':  stream << "&quot;"; break;
            }
        }

        std::tuple<std::size_t, infra::BoundedConstString> NonEscapedSubString(infra::BoundedConstString string, std::size_t start)
        {
            std::size_t escape = std::min(string.find_first_of("<>&'\"", start), string.size());
            infra::BoundedConstString nonEscapedSubString = string.substr(start, escape - start);

            for (std::size_t control = start; control != escape; ++control)
                if (string[control] < 0x20)
                {
                    escape = control;
                    nonEscapedSubString = string.substr(start, escape - start);
                    break;
                }

            return std::make_tuple(escape, nonEscapedSubString);
        }

        void InsertEscapedContent(infra::TextOutputStream& stream, infra::BoundedConstString content)
        {
            std::size_t start = 0;
            while (start != content.size())
            {
                std::size_t escape;
                infra::BoundedConstString nonEscapedSubString;
                std::tie(escape, nonEscapedSubString) = NonEscapedSubString(content, start);

                start = escape;
                if (!nonEscapedSubString.empty())
                    stream << nonEscapedSubString;
                if (escape != content.size())
                {
                    InsertPredefinedEntity(stream, content[escape]);

                    ++start;
                }
            }
        }
    }
}

namespace infra
{
    XmlFormatter::XmlFormatter(infra::TextOutputStream& stream)
        : stream(infra::inPlace, stream.Writer(), infra::noFail)
    {
        *this->stream << R"(<?xml version="1.0" encoding="ISO-8859-1" ?>)";
    }

    XmlTagFormatter XmlFormatter::Tag(const char* tagName)
    {
        return XmlTagFormatter(*stream, tagName);
    }

    XmlTagFormatter::XmlTagFormatter(infra::TextOutputStream& stream, const char* tagName)
        : stream(infra::inPlace, stream.Writer(), infra::noFail)
        , tagName(tagName)
    {
        *this->stream << "<" << tagName;
    }

    XmlTagFormatter::~XmlTagFormatter()
    {
        if (stream == infra::none)
            return;

        if (empty)
            *stream << " />";
        else
            *stream << "</" << tagName << ">";
    }

    XmlTagFormatter::XmlTagFormatter(XmlTagFormatter&& other)
        : stream(other.stream)
    {
        other.stream = infra::none;
    }

    XmlTagFormatter& XmlTagFormatter::operator=(XmlTagFormatter&& other)
    {
        stream = other.stream;
        other.stream = infra::none;

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
