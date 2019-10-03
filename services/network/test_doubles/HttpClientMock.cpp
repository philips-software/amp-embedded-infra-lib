#include "services/network/test_doubles/HttpClientMock.hpp"

namespace services
{
    void HttpHeadersEquals(const std::vector<services::HttpHeader>& expected, const services::HttpHeaders& actual)
    {
        ASSERT_EQ(expected.size(), actual.size());

        std::size_t i = 0;
        for (auto& header : actual)
            EXPECT_EQ(header, expected[i++]);
    }

    void PrintTo(const HttpHeader& header, std::ostream* stream)
    {
        infra::PrintTo(header.Field(), stream);
        *stream << ": ";
        infra::PrintTo(header.Value(), stream);
    }
}
