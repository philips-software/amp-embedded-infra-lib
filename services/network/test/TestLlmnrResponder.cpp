#include "gmock/gmock.h"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/Function.hpp"
#include "services/network/LlmnrResponder.hpp"
#include "services/network/test_doubles/AddressMock.hpp"
#include "services/network/test_doubles/DatagramMock.hpp"
#include "services/network/test_doubles/MulticastMock.hpp"

class LlmnrResponderTest
    : public testing::Test
{
public:
    LlmnrResponderTest()
        : datagramExchangePtr(infra::UnOwnedSharedPtr(datagramExchange))
        , execute([this]()
              {
                EXPECT_CALL(factory, Listen(testing::Ref(responder), 5355, services::IPVersions::ipv4)).WillOnce(testing::Return(datagramExchangePtr));
                EXPECT_CALL(multicast, JoinMulticastGroup(datagramExchangePtr, services::IPv4Address{ 224, 0, 0, 252 })); })
        , responder(factory, multicast, ipv4Info, "name")
    {}

    ~LlmnrResponderTest()
    {
        EXPECT_CALL(multicast, LeaveMulticastGroup(datagramExchangePtr, services::IPv4Address{ 224, 0, 0, 252 }));
    }

    testing::StrictMock<services::DatagramFactoryMock> factory;
    testing::StrictMock<services::MulticastMock> multicast;
    testing::StrictMock<services::IPv4InfoMock> ipv4Info;
    testing::StrictMock<services::DatagramExchangeMock> datagramExchange;
    infra::SharedPtr<services::DatagramExchange> datagramExchangePtr;
    infra::Execute execute;
    services::LlmnrResponder responder;
};

TEST_F(LlmnrResponderTest, request_record_a)
{
    std::vector<uint8_t> data{ 1, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 4, 'n', 'a', 'm', 'e', 0, 0, 1, 0, 1 };
    infra::ByteInputStreamReader requestReader(infra::MakeRange(data));

    EXPECT_CALL(datagramExchange, RequestSendStream(38, services::UdpSocket{ services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 } }));
    responder.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 });

    EXPECT_CALL(ipv4Info, GetIPv4Address()).WillOnce(testing::Return(services::IPv4Address{ 5, 6, 7, 8 }));
    infra::ByteOutputStreamWriter::WithStorage<512> response;
    responder.SendStreamAvailable(infra::UnOwnedSharedPtr(response));
    EXPECT_EQ((std::array<uint8_t, 38>{ 1, 2, 0x80, 0, 0, 1, 0, 1, 0, 0, 0, 0, 4, 'n', 'a', 'm', 'e', 0, 0, 1, 0, 1,
                  0xc0, 0x0c, 0, 1, 0, 1, 0, 0, 0, 30, 0, 4, 5, 6, 7, 8 }),
        response.Processed());
}

TEST_F(LlmnrResponderTest, query_for_someone_else_is_not_answered)
{
    std::vector<uint8_t> data{ 1, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 4, 'n', 'o', 'n', 'e', 0, 0, 1, 0, 1 };
    infra::ByteInputStreamReader requestReader(infra::MakeRange(data));

    responder.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 });
}

TEST_F(LlmnrResponderTest, short_header_is_not_answered)
{
    std::vector<uint8_t> data{ 1, 2, 0, 0, 0, 1, 0 };
    infra::ByteInputStreamReader requestReader(infra::MakeRange(data));

    responder.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 });
}

TEST_F(LlmnrResponderTest, unknown_request_is_not_answered)
{
    std::vector<uint8_t> data{ 1, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 4, 'n', 'a', 'm', 'e', 0, 0, 2, 0, 1 };
    infra::ByteInputStreamReader requestReader(infra::MakeRange(data));

    responder.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 });
}

TEST_F(LlmnrResponderTest, unknown_class_is_not_answered)
{
    std::vector<uint8_t> data{ 1, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 4, 'n', 'a', 'm', 'e', 0, 0, 1, 0, 2 };
    infra::ByteInputStreamReader requestReader(infra::MakeRange(data));

    responder.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 });
}

TEST_F(LlmnrResponderTest, unknown_flags_are_not_answered)
{
    std::vector<uint8_t> data{ 1, 2, 0, 8, 0, 1, 0, 0, 0, 0, 0, 0, 4, 'n', 'a', 'm', 'e', 0, 0, 1, 0, 1 };
    infra::ByteInputStreamReader requestReader(infra::MakeRange(data));

    responder.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 });
}

TEST_F(LlmnrResponderTest, multiple_questions_are_not_answered)
{
    std::vector<uint8_t> data{ 1, 2, 0, 1, 0, 8, 0, 0, 0, 0, 0, 0, 4, 'n', 'a', 'm', 'e', 0, 0, 1, 0, 1 };
    infra::ByteInputStreamReader requestReader(infra::MakeRange(data));

    responder.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 });
}

TEST_F(LlmnrResponderTest, improper_string_is_not_answered)
{
    std::vector<uint8_t> data{ 1, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 4, 'n', 'a', 'm', 'e', 0xa5, 0, 1, 0, 1 };
    infra::ByteInputStreamReader requestReader(infra::MakeRange(data));

    responder.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 });
}

TEST_F(LlmnrResponderTest, ignore_request_when_replying)
{
    std::vector<uint8_t> data{ 1, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 4, 'n', 'a', 'm', 'e', 0, 0, 1, 0, 1 };
    infra::ByteInputStreamReader requestReader(infra::MakeRange(data));

    EXPECT_CALL(datagramExchange, RequestSendStream(38, services::UdpSocket{ services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 } }));
    responder.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 });

    responder.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 12 });
}
