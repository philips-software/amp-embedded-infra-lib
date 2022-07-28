#include <gmock/gmock.h>
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/network/SntpClient.hpp"
#include "services/network/test_doubles/AddressMock.hpp"
#include "services/network/test_doubles/DatagramMock.hpp"
#include "services/network/test_doubles/SntpMock.hpp"

class SntpClientTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    SntpClientTest()
        : datagramExchangePtr(infra::UnOwnedSharedPtr(datagramExchange))
        , sntpClient(factory, timeWithLocalization)
        , sntpObserver(sntpClient)
    {}

    void DataReceived(const std::vector<uint8_t>& data)
    {
        infra::ByteInputStreamReader requestReader(infra::MakeRange(data));
        sntpClient.DataReceived(infra::UnOwnedSharedPtr(requestReader), services::Udpv4Socket{ { 1, 2, 3, 4 }, 123 });
    }

    void HeaderReceived(uint8_t header)
    {
        std::vector<uint8_t> data{ 0x00, 0x02, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00 };
        data.front() = header;

        DataReceived(data);
    }

    void KissOfDeathReceived(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    {
        infra::ByteOutputStreamWriter::WithStorage<512> response;
        sntpClient.SendStreamAvailable(infra::UnOwnedSharedPtr(response));
        DataReceived(std::vector<uint8_t>{ 0x24, 0x00, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, a,    b,    c,    d,
            0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00,
            0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00,
            0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00,
            0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00 });
    }

    testing::StrictMock<services::DatagramFactoryMock> factory;
    testing::StrictMock<services::DatagramExchangeMock> datagramExchange;
    infra::SharedPtr<services::DatagramExchangeMock> datagramExchangePtr;
    services::TimeWithLocalization timeWithLocalization;
    services::SntpClient sntpClient;
    testing::StrictMock<services::SntpResultObserverMock> sntpObserver;
};

TEST_F(SntpClientTest, connects_to_server)
{
    EXPECT_CALL(factory, Connect(testing::Ref(sntpClient), services::UdpSocket{ services::Udpv4Socket{ { 127, 0, 0, 1}, 123 } })).WillOnce(testing::Return(datagramExchangePtr));
    EXPECT_CALL(datagramExchange, RequestSendStream(48));

    sntpClient.RequestTime(services::IPv4Address{ 127, 0, 0, 1 });
}

TEST_F(SntpClientTest, sends_time_request)
{
    infra::ByteOutputStreamWriter::WithStorage<512> response;
    sntpClient.SendStreamAvailable(infra::UnOwnedSharedPtr(response));
    EXPECT_EQ((std::array<uint8_t, 48>{ 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00 }), response.Processed());
}

TEST_F(SntpClientTest, calls_TimeUnavailable_on_empty_response)
{
    EXPECT_CALL(sntpObserver, TimeUnavailable());
    DataReceived(std::vector<uint8_t>{});
}

TEST_F(SntpClientTest, calls_TimeUnavailable_on_too_small_response)
{
    EXPECT_CALL(sntpObserver, TimeUnavailable());
    DataReceived(std::vector<uint8_t>{ 0 });
}

TEST_F(SntpClientTest, calls_TimeUnavailble_on_invalid_header_values)
{
    EXPECT_CALL(sntpObserver, TimeUnavailable());
    HeaderReceived(0b11100100);

    EXPECT_CALL(sntpObserver, TimeUnavailable());
    HeaderReceived(0b00111100);

    EXPECT_CALL(sntpObserver, TimeUnavailable());
    HeaderReceived(0b00100111);
}

TEST_F(SntpClientTest, calls_TimeUnavailable_on_invalid_stratum)
{
    EXPECT_CALL(sntpObserver, TimeUnavailable());
    DataReceived(std::vector<uint8_t>{ 0x24, 0x10, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00 });
}

TEST_F(SntpClientTest, calls_TimeUnavailable_on_zero_transmit_timestamp)
{
    EXPECT_CALL(sntpObserver, TimeUnavailable());
    DataReceived(std::vector<uint8_t>{ 0x24, 0x02, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

TEST_F(SntpClientTest, calls_TimeUnavailable_on_mismatching_originate_timestamp)
{
    EXPECT_CALL(sntpObserver, TimeUnavailable());
    DataReceived(std::vector<uint8_t>{ 0x24, 0x02, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00 });
}

TEST_F(SntpClientTest, calls_TimeAvailable_on_valid_response)
{
    EXPECT_CALL(sntpObserver, TimeAvailable(infra::Duration{ 0 }, infra::Duration{ 0 }));

    infra::ByteOutputStreamWriter::WithStorage<512> response;
    sntpClient.SendStreamAvailable(infra::UnOwnedSharedPtr(response));
    DataReceived(std::vector<uint8_t>{ 0x24, 0x02, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00,
                                       0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00,
                                       0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00,
                                       0x83, 0xaa, 0x7e, 0x80, 0x00, 0x00, 0x00, 0x00 });
}

TEST_F(SntpClientTest, calls_KissOfDeath_on_stratum_0_response)
{
    EXPECT_CALL(sntpObserver, KissOfDeath(services::SntpResultObserver::KissCode::deny));
    KissOfDeathReceived('D', 'E', 'N', 'Y');

    EXPECT_CALL(sntpObserver, KissOfDeath(services::SntpResultObserver::KissCode::restrict));
    KissOfDeathReceived('R', 'S', 'T', 'R');

    EXPECT_CALL(sntpObserver, KissOfDeath(services::SntpResultObserver::KissCode::rateExceeded));
    KissOfDeathReceived('R', 'A', 'T', 'E');

    EXPECT_CALL(sntpObserver, TimeUnavailable());
    KissOfDeathReceived('S', 'T', 'E', 'P');
}
