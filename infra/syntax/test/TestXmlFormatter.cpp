#include "gtest/gtest.h"
#include "infra/syntax/XmlFormatter.hpp"

TEST(XmlFormatter, usage_example)
{
    infra::BoundedString::WithStorage<256> string;
    {
        infra::XmlFormatter::WithStringStream document(infra::inPlace, string);
        {
            auto note = document.Tag("note");
            {
                note.Attribute("priority", "high");
                {
                    auto to = note.Tag("to");
                    to.Content("John");
                }

                {
                    auto from = note.Tag("from");
                    from.Content("Peter");
                }

                {
                    auto heading = note.Tag("heading");
                    heading.Content("Reminder");
                }

                {
                    auto body = note.Tag("body");
                    body.Content("Don't forget to bring lunch!");
                }
            }
        }
    }

    EXPECT_EQ(R"(<?xml version="1.0" encoding="ISO-8859-1" ?><note priority="high"><to>John</to><from>Peter</from><heading>Reminder</heading><body>Don&apos;t forget to bring lunch!</body></note>)", string);
}

TEST(XmlFormatter, writes_xml_declaration_on_instantiation)
{
    infra::BoundedString::WithStorage<128> string;
    {
        infra::XmlFormatter::WithStringStream formatter(infra::inPlace, string);
    }

    EXPECT_EQ(R"(<?xml version="1.0" encoding="ISO-8859-1" ?>)", string);
}

TEST(XmlFormatter, writes_empty_element_tag_when_tag_has_no_content)
{
    infra::BoundedString::WithStorage<128> string;
    {
        infra::XmlFormatter::WithStringStream formatter(infra::inPlace, string);
        formatter.Tag("tag-name");
    }

    EXPECT_EQ(R"(<?xml version="1.0" encoding="ISO-8859-1" ?><tag-name />)", string);
}

TEST(XmlFormatter, writes_begin_and_end_tag_when_tag_has_content)
{
    infra::BoundedString::WithStorage<128> string;
    {
        infra::XmlFormatter::WithStringStream formatter(infra::inPlace, string);
        {
            auto tag = formatter.Tag("tag-name");
            tag.Content("Tag content!");
        }
    }

    EXPECT_EQ(R"(<?xml version="1.0" encoding="ISO-8859-1" ?><tag-name>Tag content!</tag-name>)", string);
}

TEST(XmlFormatter, tags_can_be_nested)
{
    infra::BoundedString::WithStorage<128> string;
    {
        infra::XmlFormatter::WithStringStream formatter(infra::inPlace, string);
        {
            auto tag = formatter.Tag("tag-name");
            auto nested = tag.Tag("nested");
        }
    }

    EXPECT_EQ(R"(<?xml version="1.0" encoding="ISO-8859-1" ?><tag-name><nested /></tag-name>)", string);
}

TEST(XmlFormatter, nested_tags_with_content)
{
    infra::BoundedString::WithStorage<128> string;
    {
        infra::XmlFormatter::WithStringStream formatter(infra::inPlace, string);
        {
            auto tag = formatter.Tag("tag");
            {
                {
                    auto one = tag.Tag("one");
                    one.Content("een");
                }

                {
                    auto two = tag.Tag("two");
                    two.Content("twee");
                }

                {
                    auto three = tag.Tag("three");
                    three.Content("drie");
                }
            }
        }
    }

    EXPECT_EQ(R"(<?xml version="1.0" encoding="ISO-8859-1" ?><tag><one>een</one><two>twee</two><three>drie</three></tag>)", string);
}

TEST(XmlFormatter, add_element_with_content)
{
    infra::BoundedString::WithStorage<128> string;
    {
        infra::XmlFormatter::WithStringStream formatter(infra::inPlace, string);
        auto root = formatter.Tag("root");
        root.Element("element", "content");
    }

    EXPECT_EQ(R"(<?xml version="1.0" encoding="ISO-8859-1" ?><root><element>content</element></root>)", string);
}

TEST(XmlFormatter, adds_attibute_to_tag)
{
    infra::BoundedString::WithStorage<128> string;
    {
        infra::XmlFormatter::WithStringStream formatter(infra::inPlace, string);
        {
            auto tag = formatter.Tag("tag-name");
            tag.Attribute("name", "value");
        }
    }

    EXPECT_EQ(R"(<?xml version="1.0" encoding="ISO-8859-1" ?><tag-name name="value" />)", string);
}
