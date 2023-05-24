#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/WebSocketClientConnectionObserver.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"
#include "gmock/gmock.h"

class WebSocketClientConnectionObserverTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{
public:
    WebSocketClientConnectionObserverTest()
    {
        connection.Attach(webSocket.Emplace("path"));
        EXPECT_CALL(connectionObserver, Attached());
        webSocket->Attach(infra::UnOwnedSharedPtr(connectionObserver));
    }

    ~WebSocketClientConnectionObserverTest() override
    {
        if (connection.IsAttached())
        {
            EXPECT_CALL(connectionObserver, Detaching());
            connection.Detach();
        }
    }

    infra::SharedPtr<infra::StreamWriter> SendData(const std::string& text)
    {
        infra::SharedPtr<infra::StreamWriter> streamWriter;
        EXPECT_CALL(connectionObserver, SendStreamAvailable(testing::_)).WillOnce(testing::SaveArg<0>(&streamWriter));
        webSocket->RequestSendStream(text.size());
        ExecuteAllActions();
        infra::TextOutputStream::WithErrorPolicy stream(*streamWriter);
        stream << text;
        return streamWriter;
    }

    infra::SharedPtr<infra::StreamReader> CheckDataReceivedWithoutAck(const std::string& data)
    {
        auto reader = webSocket->ReceiveStream();
        infra::TextInputStream::WithErrorPolicy stream(*reader);

        EXPECT_LE(data.size(), reader->Available());
        std::string receivedData(data.size(), ' ');
        infra::BoundedString boundedReceivedData(receivedData);
        stream >> boundedReceivedData;
        EXPECT_EQ(data, receivedData);

        return reader;
    }

    infra::SharedPtr<infra::StreamReader> CheckDataReceived(const std::string& data)
    {
        auto reader = CheckDataReceivedWithoutAck(data);
        webSocket->AckReceived();
        return reader;
    }

    void ExpectDataReceived(const std::string& data)
    {
        EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this, data]()
            { CheckDataReceived(data); }));
    }

    testing::StrictMock<services::ConnectionObserverFullMock> connectionObserver;
    infra::SharedOptional<services::WebSocketClientConnectionObserver> webSocket;
    testing::StrictMock<services::ConnectionStub> connection;
    infra::SharedPtr<services::Connection> connectionPtr{ infra::UnOwnedSharedPtr(connection) };
};

TEST_F(WebSocketClientConnectionObserverTest, send_frame)
{
    EXPECT_EQ(1016, webSocket->MaxSendStreamSize());
    SendData("abcd");
    EXPECT_EQ((std::vector<uint8_t>{ { 0x82, 0x84, 0, 0, 0, 0, 'a', 'b', 'c', 'd' } }), connection.sentData);
}

TEST_F(WebSocketClientConnectionObserverTest, send_two_frames)
{
    SendData("abcd");
    SendData("def");
    EXPECT_EQ((std::vector<uint8_t>{ { 0x82, 0x84, 0, 0, 0, 0, 'a', 'b', 'c', 'd', 0x82, 0x83, 0, 0, 0, 0, 'd', 'e', 'f' } }), connection.sentData);
}

TEST_F(WebSocketClientConnectionObserverTest, send_large_frame)
{
    SendData(std::string(515, 'x'));
    std::vector<uint8_t> data{ { 0x82, 0xfe, 2, 3, 0, 0, 0, 0 } };
    data.insert(data.end(), 515, 'x');
    EXPECT_EQ(data, connection.sentData);
}

TEST_F(WebSocketClientConnectionObserverTest, receive_frame)
{
    ExpectDataReceived("abcd");
    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b', 'c', 'd' } });
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, read_from_two_frames)
{
    ExpectDataReceived("abcdefg");
    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b', 'c', 'd', 0x82, 0x03, 'e', 'f', 'g' } });
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, receive_large_frame)
{
    ExpectDataReceived(std::string(515, 'x'));
    std::vector<uint8_t> data{ { 0x82, 126, 2, 3 } };
    data.insert(data.end(), 515, 'x');
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, rewind_reader)
{
    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]()
        {
        auto reader = webSocket->ReceiveStream();
        infra::TextInputStream::WithErrorPolicy stream(*reader);

        EXPECT_EQ(7, reader->Available());
        char a, b, b2, c, c2, d, d2, e, e2, f, g;
        stream >> a;
        auto save = reader->ConstructSaveMarker();
        stream >> b;
        EXPECT_EQ('a', a);
        EXPECT_EQ('b', b);

        reader->Rewind(save);
        stream >> b2;
        EXPECT_EQ('b', b2);

        stream >> c >> d >> e >> f >> g;
        EXPECT_EQ('c', c);
        EXPECT_EQ('d', d);
        EXPECT_EQ('e', e);
        EXPECT_EQ('f', f);
        EXPECT_EQ('g', g);

        reader->Rewind(save);

        stream >> b2 >> c2 >> d2 >> e2;
        EXPECT_EQ('c', c2);
        EXPECT_EQ('d', d2);
        EXPECT_EQ('e', e2); }));

    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b', 'c', 'd', 0x82, 0x03, 'e', 'f', 'g' } });
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, reconstruct_reader_after_ack)
{
    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]()
        {
        {
            auto reader = webSocket->ReceiveStream();
            infra::TextInputStream::WithErrorPolicy stream(*reader);

            EXPECT_EQ(7, reader->Available());
            char a, b, c, d, e;
            stream >> a;
            webSocket->AckReceived();
            EXPECT_EQ(6, reader->Available());

            stream >> b >> c >> d >> e;
        }

        {
            auto reader = webSocket->ReceiveStream();
            infra::TextInputStream::WithErrorPolicy stream(*reader);

            EXPECT_EQ(6, reader->Available());
            char b;
            stream >> b;
            EXPECT_EQ('b', b);

            char c, d, e;
            stream >> c >> d;
            webSocket->AckReceived();
            stream >> e;
        }

        {
            auto reader = webSocket->ReceiveStream();
            infra::TextInputStream::WithErrorPolicy stream(*reader);

            EXPECT_EQ(3, reader->Available());
            char e;
            stream >> e;
            EXPECT_EQ('e', e);
        } }));

    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b', 'c', 'd', 0x82, 0x03, 'e', 'f', 'g' } });
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, rewind_reader_after_ack)
{
    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]()
        {
        {
            auto reader = webSocket->ReceiveStream();
            infra::TextInputStream::WithErrorPolicy stream(*reader);

            EXPECT_EQ(7, reader->Available());
            char a, b;
            stream >> a;
            EXPECT_EQ('a', a);
            webSocket->AckReceived();

            stream >> b;
            EXPECT_EQ('b', b);
        }

        {
            auto reader = webSocket->ReceiveStream();
            infra::TextInputStream::WithErrorPolicy stream(*reader);

            auto save = reader->ConstructSaveMarker();
            char b, b2;
            stream >> b;
            EXPECT_EQ('b', b);
            reader->Rewind(save);

            stream >> b2;
            EXPECT_EQ('b', b2);
        } }));

    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b', 'c', 'd', 0x82, 0x03, 'e', 'f', 'g' } });
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, reconstruct_reader_after_ack_on_frame_boundary)
{
    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]()
        {
        {
            auto reader = webSocket->ReceiveStream();
            infra::TextInputStream::WithErrorPolicy stream(*reader);

            EXPECT_EQ(7, reader->Available());
            char a, b, c, d;
            stream >> a >> b >> c >> d;
            webSocket->AckReceived();
        }

        {
            auto reader = webSocket->ReceiveStream();
            infra::TextInputStream::WithErrorPolicy stream(*reader);

            EXPECT_EQ(3, reader->Available());
            char e;
            stream >> e;
            EXPECT_EQ('e', e);
        } }));

    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b', 'c', 'd', 0x82, 0x03, 'e', 'f', 'g' } });
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, DataReceived_while_reader_is_constructed)
{
    auto reader = webSocket->ReceiveStream();

    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this, &reader]()
        {
        infra::TextInputStream::WithErrorPolicy stream(*reader);

        EXPECT_EQ(7, reader->Available());
        char a;
        stream >> a;
        EXPECT_EQ('a', a); }));

    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b', 'c', 'd', 0x82, 0x03, 'e', 'f', 'g' } });
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, received_part_of_frame)
{
    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]()
        {
        auto reader = webSocket->ReceiveStream();
        infra::TextInputStream::WithErrorPolicy stream(*reader);

        EXPECT_EQ(2, reader->Available());
        char a;
        stream >> a;
        EXPECT_EQ('a', a);

        webSocket->AckReceived();
        EXPECT_EQ(1, reader->Available()); }));

    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b' } });
    ExecuteAllActions();

    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]()
        {
        auto reader = webSocket->ReceiveStream();
        infra::TextInputStream::WithErrorPolicy stream(*reader);

        EXPECT_EQ(6, reader->Available());
        char b;
        stream >> b;
        EXPECT_EQ('b', b); }));

    connection.SimulateDataReceived(std::vector<uint8_t>{ { 'c', 'd', 0x82, 0x03, 'e', 'f', 'g' } });
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, Peek)
{
    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]()
        {
        auto reader = webSocket->ReceiveStream();
        infra::TextInputStream::WithErrorPolicy stream(*reader);

        EXPECT_EQ('a', reader->Peek(stream.ErrorPolicy()));
        char a;
        stream >> a;
        EXPECT_EQ('a', a);
        EXPECT_EQ('b', reader->Peek(stream.ErrorPolicy())); }));

    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b', 'c', 'd', 0x82, 0x03, 'e', 'f', 'g' } });
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, PeekContiguousRange)
{
    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]()
        {
        auto reader = webSocket->ReceiveStream();

        EXPECT_EQ("abcd", infra::ByteRangeAsString(reader->PeekContiguousRange(0)));
        EXPECT_EQ("bcd", infra::ByteRangeAsString(reader->PeekContiguousRange(1)));
        EXPECT_EQ("efg", infra::ByteRangeAsString(reader->PeekContiguousRange(4)));
        EXPECT_EQ("fg", infra::ByteRangeAsString(reader->PeekContiguousRange(5))); }));

    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b', 'c', 'd', 0x82, 0x03, 'e', 'f', 'g' } });
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, ExtractContiguousRange)
{
    EXPECT_CALL(connectionObserver, DataReceived()).WillOnce(testing::Invoke([this]()
        {
        auto reader = webSocket->ReceiveStream();

        EXPECT_EQ("ab", infra::ByteRangeAsString(reader->ExtractContiguousRange(2)));
        EXPECT_EQ("cd", infra::ByteRangeAsString(reader->ExtractContiguousRange(10)));
        EXPECT_EQ("efg", infra::ByteRangeAsString(reader->ExtractContiguousRange(10))); }));

    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b', 'c', 'd', 0x82, 0x03, 'e', 'f', 'g' } });
    ExecuteAllActions();
}

TEST_F(WebSocketClientConnectionObserverTest, send_and_receive_ping)
{
    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x89, 0x04, 'p', 'i', 'n', 'g' } });
    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ { 0x8a, 0x84, 0, 0, 0, 0, 'p', 'i', 'n', 'g' } }), connection.sentData);
}

TEST_F(WebSocketClientConnectionObserverTest, send_and_receive_ping_in_two_buffers)
{
    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x89, 0x04, 'p', 'i', 'n' } });
    ExecuteAllActions();
    connection.SimulateDataReceived(std::vector<uint8_t>{ 'g' });
    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ { 0x8a, 0x84, 0, 0, 0, 0, 'p', 'i', 'n', 'g' } }), connection.sentData);
}

TEST_F(WebSocketClientConnectionObserverTest, send_and_receive_ping_between_data)
{
    ExpectDataReceived("abcdefg");
    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x82, 0x04, 'a', 'b', 'c', 'd', 0x89, 0x04, 'p', 'i', 'n', 'g', 0x82, 0x03, 'e', 'f', 'g' } });
    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ { 0x8a, 0x84, 0, 0, 0, 0, 'p', 'i', 'n', 'g' } }), connection.sentData);
}

TEST_F(WebSocketClientConnectionObserverTest, last_ping_is_ponged)
{
    connection.SimulateDataReceived(std::vector<uint8_t>{ { 0x89, 0x04, 'p', 'i', 'n', 'g', 0x89, 0x04, 'p', 'i', 'n', '2' } });
    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ { 0x8a, 0x84, 0, 0, 0, 0, 'p', 'i', 'n', '2' } }), connection.sentData);
}

TEST_F(WebSocketClientConnectionObserverTest, CloseAndDestroy)
{
    EXPECT_CALL(connectionObserver, Detaching());
    EXPECT_CALL(connection, CloseAndDestroyMock());
    webSocket->CloseAndDestroy();
}

TEST_F(WebSocketClientConnectionObserverTest, AbortAndDestroy)
{
    EXPECT_CALL(connectionObserver, Detaching());
    EXPECT_CALL(connection, AbortAndDestroyMock());
    webSocket->AbortAndDestroy();
}
