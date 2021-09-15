#include "infra/util/Tokenizer.hpp"
#include "gmock/gmock.h"

TEST(TokenizerTest, construction)
{
    infra::BoundedConstString string = "abcd efgh";
    infra::Tokenizer tokenizer(string, ' ');
}

TEST(TokenizerTest, get_single_token)
{
    infra::BoundedConstString string = "abcd";
    infra::Tokenizer tokenizer(string, ' ');

    EXPECT_EQ("abcd", tokenizer.Token(0));
}

TEST(TokenizerTest, get_token_out_of_many)
{
    infra::BoundedConstString string = "abcd defg hijk";
    infra::Tokenizer tokenizer(string, ' ');

    EXPECT_EQ("abcd", tokenizer.Token(0));
    EXPECT_EQ("defg", tokenizer.Token(1));
    EXPECT_EQ("hijk", tokenizer.Token(2));
}

TEST(TokenizerTest, get_number_of_tokens)
{
    infra::BoundedConstString string = "abcd defg hijk";
    infra::Tokenizer tokenizer(string, ' ');

    EXPECT_EQ(3, tokenizer.Size());
}

TEST(TokenizerTest, get_number_of_tokens_of_empty_string)
{
    infra::BoundedConstString string = " ";
    infra::Tokenizer tokenizer(string, ' ');

    EXPECT_EQ(0, tokenizer.Size());
}

TEST(TokenizerTest, nonexisting_token_is_empty)
{
    infra::BoundedConstString string = "abcd defg hijk";
    infra::Tokenizer tokenizer(string, ' ');

    EXPECT_EQ("", tokenizer.Token(3));
    EXPECT_EQ("", tokenizer.Token(4));
}

TEST(TokenizerTest, skip_leading_tokens)
{
    infra::BoundedConstString string = "  abcd defg hijk";
    infra::Tokenizer tokenizer(string, ' ');

    EXPECT_EQ("abcd", tokenizer.Token(0));
}

TEST(TokenizerTest, skip_middle_tokens)
{
    infra::BoundedConstString string = "  abcd    defg hijk";
    infra::Tokenizer tokenizer(string, ' ');

    EXPECT_EQ("defg", tokenizer.Token(1));
}

TEST(TokenizerTest, skip_end_tokens)
{
    infra::BoundedConstString string = "abcd defg hijk  ";
    infra::Tokenizer tokenizer(string, ' ');

    EXPECT_EQ("", tokenizer.Token(3));
}

TEST(TokenizerTest, token_from_colon_separated_string)
{
    infra::BoundedConstString string = "abcd:defg:hijk";
    infra::Tokenizer tokenizer(string, ':');

    EXPECT_EQ("defg", tokenizer.Token(1));
}

TEST(TokenizerTest, get_second_TokenAndRest)
{
    infra::BoundedConstString string = "abcd defg hijk";
    infra::Tokenizer tokenizer(string, ' ');

    EXPECT_EQ("defg hijk", tokenizer.TokenAndRest(1));
    EXPECT_EQ("", tokenizer.TokenAndRest(3));
}
