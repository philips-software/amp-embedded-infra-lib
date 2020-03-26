#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/Function.hpp"
#include "services/network/DnsServer.hpp"
#include "services/network/test_doubles/AddressMock.hpp"
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
        server.DataReceived(requestReader, services::Udpv4Socket{ { 1, 2, 3, 4 }, 53 });
    }

    void ExpectResponse(const std::vector<uint8_t>& data)
    {
        infra::ByteOutputStreamWriter::WithStorage<512> response;
        server.SendStreamAvailable(infra::UnOwnedSharedPtr(response));
        EXPECT_EQ(data, response.Processed());
    }

    testing::StrictMock<services::DatagramFactoryMock> factory;
    testing::StrictMock<services::DatagramExchangeMock> datagramExchange;
    testing::StrictMock<services::IPv4InfoMock> ipv4Info;
    infra::SharedPtr<services::DatagramExchange> datagramExchangePtr{ infra::UnOwnedSharedPtr(datagramExchange) };
    infra::Execute execute;
    services::DnsServer server{ factory, ipv4Info };
};

TEST_F(DnsServerTest, should_respond_to_request_with_own_address)
{
    EXPECT_CALL(datagramExchange, RequestSendStream(46, services::UdpSocket{ services::Udpv4Socket{ { 1, 2, 3, 4 }, 53 } }));
    DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });

    EXPECT_CALL(ipv4Info, GetIPv4Address()).WillOnce(testing::Return(services::IPv4Address{ 5, 6, 7, 8 }));
    ExpectResponse({ 10, 10, 128, 0, 0, 1, 0, 1, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0,
        0, 1, 0, 1, 0xC0, 12, 0, 1, 0, 1, 0, 0, 0, 60, 0, 4, 5, 6, 7, 8 });
}

TEST_F(DnsServerTest, should_respond_to_multiple_consecutive_questions)
{
    for (int i = 0; i < 2; ++i)
    {
        EXPECT_CALL(datagramExchange, RequestSendStream(46, services ::UdpSocket{ services::Udpv4Socket{ { 1, 2, 3, 4 }, 53 } }));
        DataReceived({ 10, 10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 8, 'h', 'o', 's', 't', 'n', 'a', 'm', 'e', 3, 'c', 'o', 'm', 0, 0, 1, 0, 1 });

        EXPECT_CALL(ipv4Info, GetIPv4Address()).WillOnce(testing::Return(services::IPv4Address{ 5, 6, 7, 8 }));
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
