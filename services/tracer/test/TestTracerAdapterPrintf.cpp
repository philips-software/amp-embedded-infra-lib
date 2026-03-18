#include "infra/stream/StringOutputStream.hpp"
#include "services/tracer/TracerAdapterPrintf.hpp"
#include "gmock/gmock.h"

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
    services::TracerToStream tracer;
    services::TracerAdapterPrintf adapter;
};

TEST_F(TracerAdapterPrintfTest, print_newline_to_tracer)
{
    Print("\n");
    EXPECT_EQ("\r\n", stream.Storage());
}

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

TEST_F(TracerAdapterPrintfTest, print_int_in_hex_with_width_containing_zero_to_tracer)
{
    Print("%10x", 0xABCD);
    EXPECT_EQ("      abcd", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_int_in_hex_with_width_padding_0_to_tracer)
{
    Print("%08x", 0xABCD);
    EXPECT_EQ("0000abcd", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_long_long_in_hex_with_width_padding_0_to_tracer)
{
    Print("%016llx", 0xABCDLL);
    EXPECT_EQ("000000000000abcd", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_int_in_hex_with_X_to_tracer)
{
    Print("%X", 0x123A);
    EXPECT_EQ("123a", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_pointer_with_p_to_tracer)
{
    Print("%p", 0x123A);
    EXPECT_EQ("0x123a", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_multiple_values)
{
    Print("%u%c%s", 100, '2', "300");
    EXPECT_EQ("1002300", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_string_with_dynamic_precision)
{
    const char* str = "No admittance except on party business";
    Print("%.*s", 13, str);
    EXPECT_EQ("No admittance", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_string_with_static_precision)
{
    Print("%.5s", "Hello, Tracer!");
    EXPECT_EQ("Hello", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_string_with_zero_precision)
{
    Print("%.0s", "Hello, Tracer!");
    EXPECT_EQ("", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_string_with_dynamic_zero_precision)
{
    Print("%.*s", 0, "Hello, Tracer!");
    EXPECT_EQ("", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_string_with_precision_larger_than_string)
{
    Print("%.*s", 100, "Hello");
    EXPECT_EQ("Hello", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_null_string_with_precision)
{
    Print("%.*s", 5, nullptr);
    EXPECT_EQ("(null)", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_string_with_dynamic_precision_ignores_width)
{
    // Width is not implemented for strings
    Print("%10.*s", 5, "Hello, Tracer!");
    EXPECT_EQ("Hello", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, print_string_with_static_precision_ignores_width)
{
    // Width is not implemented for strings
    Print("%10.5s", "Hello, Tracer!");
    EXPECT_EQ("Hello", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, trailing_percent_does_not_read_out_of_bounds)
{
    Print("hello%");
    EXPECT_EQ("hello", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, lone_trailing_percent_does_not_read_out_of_bounds)
{
    Print("%");
    EXPECT_EQ("", stream.Storage());
}

TEST_F(TracerAdapterPrintfTest, trailing_percent_after_valid_format_does_not_read_out_of_bounds)
{
    Print("%d%", 42);
    EXPECT_EQ("42", stream.Storage());
}
