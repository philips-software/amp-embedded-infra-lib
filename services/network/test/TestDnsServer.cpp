#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/Function.hpp"
#include "services/network/DnsServer.hpp"
#include "services/network/test_doubles/DatagramMock.hpp"
#include "gmock/gmock.h"

class DnsServerTest
    : public testing::Test
{
public:
    DnsServerTest()
        : execute([this] {
            EXPECT_CALL(factory, Listen(testing::Ref(server), 53, services::IPVersions::ipv4)).WillOnce(testing::Return(datagramExchangePtr));
        })
    {}

    void DataReceived(const std::vector<uint8_t>& data)
    {
        infra::ByteInputStreamReader requestReader(infra::MakeRange(data));
        server.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 53 });
    }

    void ExpectResponse(const std::vector<uint8_t>& data)
    {
        infra::ByteOutputStreamWriter::WithStorage<512> response;
        server.SendStreamAvailable(infra::UnOwnedSharedPtr(response));
        EXPECT_EQ(data, response.Processed());
    }

    testing::StrictMock<services::DatagramFactoryMock> factory;
    testing::StrictMock<services::DatagramExchangeMock> datagramExchange;
    infra::SharedPtr<services::DatagramExchange> datagramExchangePtr{ infra::UnOwnedSharedPtr(datagramExchange) };
    infra::Execute execute;
    std::vector<services::DnsServer::DnsEntry> entries{
        { "hostname.com", services::IPv4Address{ 5, 6, 7, 8 } },
        { "other.com", services::IPv4Address{ 9, 10, 11, 12 } }
    };
    services::DnsServerImpl server{ factory, services::DnsServer::DnsEntries{ entries } };
};

TEST_F(DnsServerTest, should_not_respond_to_question_with_unknown_hostname)
{
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 7, 'u', 'n', 'k', 'n', 'o', 'w', 'n', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });
}

TEST_F(DnsServerTest, should_respond_to_question_with_corresponding_address)
{
    EXPECT_CALL(datagramExchange, RequestSendStream(46, services::UdpSocket{ services::Udpv4Socket{ { 1, 2, 3, 4 }, 53 } }));
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });

    ExpectResponse({ 10, 10, 128, 0, 0, 1, 0, 1, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0,
        0, 1, 0, 1, 0xC0, 12, 0, 1, 0, 1, 0, 0, 0, 60, 0, 4, 5, 6, 7, 8 });
}

TEST_F(DnsServerTest, should_respond_to_question_for_other_hostname)
{
    EXPECT_CALL(datagramExchange, RequestSendStream(43, services::UdpSocket{ services::Udpv4Socket{ { 1, 2, 3, 4 }, 53 } }));
    DataReceived({ 20, 20, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5, 'o', 't', 'h', 'e', 'r', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });

    ExpectResponse({ 20, 20, 128, 0, 0, 1, 0, 1, 0, 0, 0, 0, 5, 'o', 't', 'h', 'e', 'r', 3, 'c', 'o', 'm', 0,
        0, 1, 0, 1, 0xC0, 12, 0, 1, 0, 1, 0, 0, 0, 60, 0, 4, 9, 10, 11, 12 });
}

TEST_F(DnsServerTest, should_compare_hostname_case_insensitive)
{
    EXPECT_CALL(datagramExchange, RequestSendStream(46, services::UdpSocket{ services::Udpv4Socket{ { 1, 2, 3, 4 }, 53 } }));
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 8, 'H', 'o', 's', 't', 'N', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });

    ExpectResponse({ 10, 10, 128, 0, 0, 1, 0, 1, 0, 0, 0, 0, 8, 'H', 'o', 's', 't', 'N', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0,
        0, 1, 0, 1, 0xC0, 12, 0, 1, 0, 1, 0, 0, 0, 60, 0, 4, 5, 6, 7, 8 });
}

TEST_F(DnsServerTest, should_strip_www_from_hostname_before_compare)
{
    EXPECT_CALL(datagramExchange, RequestSendStream(50, services::UdpSocket{ services::Udpv4Socket{ { 1, 2, 3, 4 }, 53 } }));
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 3, 'w', 'w', 'w', 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });

    ExpectResponse({ 10, 10, 128, 0, 0, 1, 0, 1, 0, 0, 0, 0, 3, 'w', 'w', 'w', 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0,
        0, 1, 0, 1, 0xC0, 12, 0, 1, 0, 1, 0, 0, 0, 60, 0, 4, 5, 6, 7, 8 });
}

TEST_F(DnsServerTest, should_not_strip_www_from_end_of_hostname)
{
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'w', 'w', 'w', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });
}

TEST_F(DnsServerTest, should_respond_to_multiple_consecutive_questions)
{
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_CALL(datagramExchange, RequestSendStream(46, services ::UdpSocket{ services::Udpv4Socket{ { 1, 2, 3, 4 }, 53 } }));
        DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });

        ExpectResponse({ 10, 10, 128, 0, 0, 1, 0, 1, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0,
            0, 1, 0, 1, 0xC0, 12, 0, 1, 0, 1, 0, 0, 0, 60, 0, 4, 5, 6, 7, 8 });
    }
}

TEST_F(DnsServerTest, should_not_accept_request_when_already_in_progress)
{
    EXPECT_CALL(datagramExchange, RequestSendStream(46, services ::UdpSocket{ services::Udpv4Socket{ { 1, 2, 3, 4 }, 53 } }));
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });

    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });
}

TEST_F(DnsServerTest, should_not_accept_request_with_incorrect_counts)
{
    DataReceived({ 10, 10, 0, 0, 0xF, 0xF, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });
    DataReceived({ 10, 10, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });
}

TEST_F(DnsServerTest, should_not_accept_request_with_incorrect_type)
{
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 0xF, 0, 1 });
}

TEST_F(DnsServerTest, should_not_accept_request_with_incorrect_class)
{
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 0xF });
}

TEST_F(DnsServerTest, should_not_accept_request_with_incorrect_opcode)
{
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 0xF });
}

TEST_F(DnsServerTest, should_not_accept_short_request)
{
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 });
}
