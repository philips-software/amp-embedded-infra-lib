#include "gmock/gmock.h"
#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/WebSocketServerConnectionObserver.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"

class WebSocketServerConnectionObserverTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{
public:
    WebSocketServerConnectionObserverTest()
    {
        connection.Attach(webSocket.Emplace());
        EXPECT_CALL(connectionObserver, Attached());
        webSocket->Attach(infra::UnOwnedSharedPtr(connectionObserver));
    }

    ~WebSocketServerConnectionObserverTest()
    {
        if (connection.IsAttached())
        {
            EXPECT_CALL(connectionObserver, Detaching());
            connection.Detach();
        }
    }

    infra::SharedPtr<infra::StreamWriter> SendData(const std::vector<uint8_t>& dataToSend, std::size_t requestSize = 2)
    {
        infra::SharedPtr<infra::StreamWriter> streamWriter;
        EXPECT_CALL(connectionObserver, SendStreamAvailable(testing::_)).WillOnce(testing::SaveArg<0>(&streamWriter));
        webSocket->RequestSendStream(requestSize);
        infra::DataOutputStream::WithErrorPolicy stream(*streamWriter);
        stream << infra::MakeRange(dataToSend);
        return streamWriter;
    }

    infra::SharedPtr<infra::StreamReader> CheckDataReceivedWithoutAck(infra::ConstByteRange data)
    {
        auto reader = webSocket->ReceiveStream();
        infra::DataInputStream::WithErrorPolicy stream(*reader);

        std::vector<uint8_t> receivedData(data.size(), 0);
        stream >> infra::MakeRange(receivedData);
        EXPECT_EQ(data, receivedData);

        return reader;
    }

    infra::SharedPtr<infra::StreamReader> CheckDataReceived(const std::vector<uint8_t>& data)
    {
        auto reader = CheckDataReceivedWithoutAck(data);
        webSocket->AckReceived();
        return reader;
    }

    void ExpectDataReceived(const std::vector<uint8_t>& data)
    {
        EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this, data]() {
            CheckDataReceived(data);
        }));
    }

    testing::StrictMock<services::ConnectionObserverFullMock> connectionObserver;
    infra::SharedOptional<services::WebSocketServerConnectionObserver::WithBufferSizes<512, 512>> webSocket;
    testing::StrictMock<services::ConnectionStub> connection;
    infra::SharedPtr<services::Connection> connectionPtr{ infra::UnOwnedSharedPtr(connection) };
};

TEST_F(WebSocketServerConnectionObserverTest, MaxSendStreamSize)
{
    EXPECT_EQ(512, webSocket->MaxSendStreamSize());
}

TEST_F(WebSocketServerConnectionObserverTest, CloseAndDestroy_closes_websocket)
{
    EXPECT_CALL(connection, CloseAndDestroyMock());
    EXPECT_CALL(connectionObserver, Detaching());
    webSocket->CloseAndDestroy();
}

TEST_F(WebSocketServerConnectionObserverTest, AbortAndDestroy_closes_websocket)
{
    EXPECT_CALL(connection, AbortAndDestroyMock());
    EXPECT_CALL(connectionObserver, Detaching());
    webSocket->AbortAndDestroy();
}

TEST_F(WebSocketServerConnectionObserverTest, forward_single_frame)
{
    std::array<uint8_t, 11> receiveData = { 0x82, 0x85, 0xa5, 0xb5, 0xc5, 0xd5, 0x34, 0x63, 0xa5, 0x7b, 0xc9 };

    ExpectDataReceived({ 0x91, 0xd6, 0x60, 0xae, 0x6c });
    connection.SimulateDataReceived(receiveData);
}

TEST_F(WebSocketServerConnectionObserverTest, frame_header_not_received_in_one_go)
{
    std::array<uint8_t, 1> receiveData1 = { 0x82 };
    std::array<uint8_t, 10> receiveData2 = { 0x85, 0xa5, 0xb5, 0xc5, 0xd5, 0x34, 0x63, 0xa5, 0x7b, 0xc9 };

    connection.SimulateDataReceived(receiveData1);
    ExpectDataReceived({ 0x91, 0xd6, 0x60, 0xae, 0x6c });
    connection.SimulateDataReceived(receiveData2);
}

TEST_F(WebSocketServerConnectionObserverTest, frame_payload_not_received_in_one_go)
{
    std::array<uint8_t, 8> receiveData1 = { 0x82, 0x85, 0xa5, 0xb5, 0xc5, 0xd5, 0x34, 0x63 };
    std::array<uint8_t, 2> receiveData2 = { 0xa5, 0x7b };
    std::array<uint8_t, 1> receiveData3 = { 0xc9 };

    EXPECT_CALL(connectionObserver, DataReceived());
    connection.SimulateDataReceived(receiveData1);
    EXPECT_CALL(connectionObserver, DataReceived());
    connection.SimulateDataReceived(receiveData2);
    ExpectDataReceived({ 0x91, 0xd6, 0x60, 0xae, 0x6c });
    connection.SimulateDataReceived(receiveData3);
}

TEST_F(WebSocketServerConnectionObserverTest, two_separate_frames)
{
    std::array<uint8_t, 14> receiveData = { 0x82, 0x81, 0xa5, 0xb5, 0xc5, 0xd5, 0x34, 0x82, 0x81, 0xa5, 0xb5, 0xc5, 0xd5, 0x34 };

    testing::InSequence s;
    ExpectDataReceived({ 0x91 });
    connection.SimulateDataReceived(receiveData);
    CheckDataReceived({ 0x91 });
}

TEST_F(WebSocketServerConnectionObserverTest, two_continuous_frames_belongs_one_data)
{
    std::array<uint8_t, 14> receiveData = { 0x02, 0x81, 0xa5, 0xb5, 0xc5, 0xd5, 0x34, 0x80, 0x81, 0xa5, 0xb5, 0xc5, 0xd5, 0x34 };

    testing::InSequence s;
    ExpectDataReceived({ 0x91 });
    connection.SimulateDataReceived(receiveData);
    CheckDataReceived({ 0x91 });
}

TEST_F(WebSocketServerConnectionObserverTest, one_full_one_partial_frames_separate_in_header)
{
    std::array<uint8_t, 9> receiveData1 = { 0x82, 0x81, 0xa5, 0xb5, 0xc5, 0xd5, 0x34, 0x82, 0x81 };
    std::array<uint8_t, 5> receiveData2 = { 0xa5, 0xb5, 0xc5, 0xd5, 0x34 };

    ExpectDataReceived({ 0x91 });
    connection.SimulateDataReceived(receiveData1);

    ExpectDataReceived({ 0x91 });
    connection.SimulateDataReceived(receiveData2);
}

TEST_F(WebSocketServerConnectionObserverTest, one_full_one_partial_frames_separate_in_data)
{
    std::array<uint8_t, 15> receiveData1 = { 0x82, 0x82, 0xa5, 0xa5, 0xa5, 0xa5, 0x34, 0x34, 0x82, 0x82, 0xa5, 0xa5, 0xa5, 0xa5, 0x34 };
    std::array<uint8_t, 1> receiveData2 = { 0x34 };

    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]() {
        CheckDataReceived({ 0x91, 0x91 });
        CheckDataReceived({ 0x91 });
    }));
    connection.SimulateDataReceived(receiveData1);

    ExpectDataReceived({ 0x91 });
    connection.SimulateDataReceived(receiveData2);
}

TEST_F(WebSocketServerConnectionObserverTest, two_partial_in_three_chunks)
{
    std::array<uint8_t, 3> receiveData1 = { 0x82, 0x82, 0xa5 };
    std::array<uint8_t, 12> receiveData2 = { 0xa5, 0xa5, 0xa5, 0x34, 0x34, 0x82, 0x82, 0xa5, 0xa5, 0xa5, 0xa5, 0x34 };
    std::array<uint8_t, 1> receiveData3 = { 0x34 };

    connection.SimulateDataReceived(receiveData1);

    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]() {
        CheckDataReceived({ 0x91, 0x91 });
        CheckDataReceived({ 0x91 });
    }));
    connection.SimulateDataReceived(receiveData2);

    ExpectDataReceived({ 0x91 });
    connection.SimulateDataReceived(receiveData3);
}

TEST_F(WebSocketServerConnectionObserverTest, receive_frame_with_larger_payload_than_receive_buffer)
{
    std::array<uint8_t, 8> receiveDataHeader = { 0x82, 0xFE, 0x02, 0x08, 0xa5, 0xa5, 0xa5, 0xa5 };
    std::array<uint8_t, 20> receiveDataPayload = { 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34 };

    connection.SimulateDataReceived(receiveDataHeader);
    for (int i = 0; i != 25; ++i)
    {
        EXPECT_CALL(connectionObserver, DataReceived());
        connection.SimulateDataReceived(receiveDataPayload);
    }

    testing::InSequence s;
    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]() {
        CheckDataReceived(std::vector<uint8_t>(512, 0x91));
        CheckDataReceived(std::vector<uint8_t>(8, 0x91));
    }));

    connection.SimulateDataReceived(receiveDataPayload);
}

TEST_F(WebSocketServerConnectionObserverTest, receive_close_request_from_client)
{
    std::array<uint8_t, 11> receiveData = { 0x88, 0x85, 0xa5, 0xb5, 0xc5, 0xd5, 0x34, 0x63, 0xa5, 0x7b, 0xc9 };
    std::vector<uint8_t> sendData = { 0x88, 0x00 };

    connection.SimulateDataReceived(receiveData);
    EXPECT_CALL(connection, CloseAndDestroyMock()).WillOnce(testing::Invoke([this, &sendData]() { EXPECT_EQ(sendData, connection.sentData); }));
    EXPECT_CALL(connectionObserver, Detaching());
    ExecuteAllActions();
}

TEST_F(WebSocketServerConnectionObserverTest, receive_wrong_operation_code)
{
    std::array<uint8_t, 7> receiveData = { 0x85, 0x81, 0xa5, 0xb5, 0xc5, 0xd5, 0x34 };
    std::vector<uint8_t> sendData = { 0x88, 0x00 };

    connection.SimulateDataReceived(receiveData);
    EXPECT_CALL(connection, CloseAndDestroyMock()).WillOnce(testing::Invoke([this, &sendData]() { EXPECT_EQ(sendData, connection.sentData); }));
    EXPECT_CALL(connectionObserver, Detaching());
    ExecuteAllActions();
}

TEST_F(WebSocketServerConnectionObserverTest, receive_unmasked_frame_from_client)
{
    std::array<uint8_t, 7> receiveData = { 0x82, 0x01, 0xa5, 0xb5, 0xc5, 0xd5, 0x34 };
    std::vector<uint8_t> sendData = { 0x88, 0x00 };

    connection.SimulateDataReceived(receiveData);
    EXPECT_CALL(connection, CloseAndDestroyMock()).WillOnce(testing::Invoke([this, &sendData]() { EXPECT_EQ(sendData, connection.sentData); }));
    EXPECT_CALL(connectionObserver, Detaching());
    ExecuteAllActions();
}

TEST_F(WebSocketServerConnectionObserverTest, receive_non_zero_rsv)
{
    std::array<uint8_t, 7> receiveData = { 0x92, 0x81, 0xa5, 0xb5, 0xc5, 0xd5, 0x34 };
    std::vector<uint8_t> sendData = { 0x88, 0x00 };

    connection.SimulateDataReceived(receiveData);
    EXPECT_CALL(connection, CloseAndDestroyMock()).WillOnce(testing::Invoke([this, &sendData]() { EXPECT_EQ(sendData, connection.sentData); }));
    EXPECT_CALL(connectionObserver, Detaching());
    ExecuteAllActions();
}

TEST_F(WebSocketServerConnectionObserverTest, receive_ping_request)
{
    std::array<uint8_t, 11> receiveData = { 0x89, 0x85, 0xa5, 0xb5, 0xc5, 0xd5, 0x34, 0x63, 0xa5, 0x7b, 0xc9 };
    std::vector<uint8_t> sendFrame = { 0x8A, 0x05, 0x91, 0xd6, 0x60, 0xae, 0x6c };

    connection.SimulateDataReceived(receiveData);
    ExecuteAllActions();
    EXPECT_EQ(sendFrame, connection.sentData);
}

TEST_F(WebSocketServerConnectionObserverTest, send_data_in_one_frame)
{
    std::vector<uint8_t> sendFrame = { 0x82, 0x02, 0x91, 0x91 };

    SendData({ 0x91, 0x91 });
    ExecuteAllActions();
    EXPECT_EQ(sendFrame, connection.sentData);
}

TEST_F(WebSocketServerConnectionObserverTest, send_data_after_first_frame)
{
    std::vector<uint8_t> sendFrame = { 0x82, 0x02, 0x91, 0x91 };

    {
        auto writer = SendData({ 0x91, 0x91 });
        ExecuteAllActions();
    }

    webSocket->RequestSendStream(2);

    EXPECT_CALL(connectionObserver, SendStreamAvailable(testing::_)).WillOnce(testing::Invoke([this](infra::SharedPtr<infra::StreamWriter> writer) {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);
        std::vector<uint8_t> dataToSend{ 0x91, 0x91 };
        stream << infra::MakeRange(dataToSend);
    }));
    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0x82, 0x02, 0x91, 0x91, 0x82, 0x02, 0x91, 0x91 }), connection.sentData);
}

TEST_F(WebSocketServerConnectionObserverTest, dont_use_whole_buffer)
{
    std::vector<uint8_t> sendFrame = { 0x82, 0x02, 0x91, 0x91 };

    SendData({ 0x91, 0x91 }, 128);
    ExecuteAllActions();
    EXPECT_EQ(sendFrame, connection.sentData);
}
