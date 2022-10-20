#include "infra/stream/StringOutputStream.hpp"
#include "services/tracer/TracerAdapterPrintf.hpp"
#include <gmock/gmock.h>

services::TracerAdapterPrintf* printfAdapter = nullptr;

void Print(const char* format, ...)
{
    ASSERT_TRUE(printfAdapter != nullptr);

    va_list args;

    va_start(args, format);
    printfAdapter->Print(format, &args);
    va_end(args);
}

class TracerAdapterPrintfTest
    : public testing::Test
{
public:
    TracerAdapterPrintfTest()
        : tracer(stream)
        , adapter(tracer)
    {
        printfAdapter = &adapter;
    }

    infra::StringOutputStream::WithStorage<32> stream;
    services::Tracer tracer;
    services::TracerAdapterPrintf adapter;
};

TEST_F(TracerAdapterPrintfTest, print_format_string_to_tracer)
{
    Print("Hello, Tracer!");
    EXPECT_EQ("Hello, Tracer!", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_escaped_percent_sign_to_tracer)
{
    Print("%%");
    EXPECT_EQ("%", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_unknown_format_string_to_tracer)
{
    Print("%llq");
    EXPECT_EQ("llq", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_character_to_tracer)
{
    Print("%c", 'A');
    EXPECT_EQ("A", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_string_to_tracer)
{
    Print("%s", "Hello, Tracer!");
    EXPECT_EQ("Hello, Tracer!", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_null_string_to_tracer)
{
    Print("%s", nullptr);
    EXPECT_EQ("(null)", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_int_to_tracer)
{
    Print("%d", -42);
    EXPECT_EQ("-42", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_long_to_tracer)
{
    Print("%ld", -42L);
    EXPECT_EQ("-42", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_long_with_width_to_tracer)
{
    Print("%08ld", -42L);
    EXPECT_EQ("-42", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_long_long_to_tracer)
{
    Print("%lld", -42LL);
    EXPECT_EQ("-42", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_float_to_tracer)
{
    Print("%f", 42.123f);
    EXPECT_EQ("42.123", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_int_in_hex_with_x_to_tracer)
{
    Print("%x", 0xABCD);
    EXPECT_EQ("abcd", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_int_in_hex_with_width_to_tracer)
{
    Print("%8x", 0xABCD);
    EXPECT_EQ("    abcd", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_int_in_hex_with_width_padding_0_to_tracer)
{
    Print("%08x", 0xABCD);
    EXPECT_EQ("0000abcd", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_int_in_hex_with_X_to_tracer)
{
    Print("%X", 0x123A);
    EXPECT_EQ("123a", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_multiple_values)
{
    Print("%u%c%s", 100, '2', "300");
    EXPECT_EQ("1002300", stream.Storage());
}
