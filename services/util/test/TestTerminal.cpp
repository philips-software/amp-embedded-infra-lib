#include "gmock/gmock.h"
#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/util/Terminal.hpp"
#include <vector>

class StreamWriterMock
    : public infra::StreamWriter
{
public:
    using StreamWriter::StreamWriter;

    MOCK_METHOD2(Insert, void(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy));
    MOCK_CONST_METHOD0(Available, std::size_t());
    MOCK_CONST_METHOD0(ConstructSaveMarker, std::size_t());
    MOCK_CONST_METHOD1(GetProcessedBytesSince, std::size_t(std::size_t marker));
    MOCK_METHOD1(SaveState, infra::ByteRange(std::size_t marker));
    MOCK_METHOD1(RestoreState, void(infra::ByteRange range));
    MOCK_METHOD1(Overwrite, infra::ByteRange(std::size_t marker));
};

class TerminalTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{
public:
    TerminalTest()
        : stream(streamWriterMock)
        , tracer(stream)
    {
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));
        terminal.Emplace(communication, tracer);
    }

protected:
    StreamWriterMock streamWriterMock;
    infra::TextOutputStream::WithErrorPolicy stream;
    services::Tracer tracer;
    testing::StrictMock<hal::SerialCommunicationMock> communication;
    infra::Optional<services::Terminal> terminal;
    testing::InSequence s;
};

class TerminalNavigationTest
    : public TerminalTest
{
public:
    TerminalNavigationTest()
    {
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'a' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'b' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'c' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'd' } }), testing::_));

        communication.dataReceived(std::vector<uint8_t>{ 'a', 'b', 'c', 'd' });
    }
};

class TerminalHistoryTest
    : public TerminalTest
{
public:
    TerminalHistoryTest()
    {
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'a' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'b' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'c' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'd' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r', '\n' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));

        communication.dataReceived(std::vector<uint8_t>{ 'a', 'b', 'c', 'd', '\r' });

        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'e' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'f' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'g' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r', '\n' } }), testing::_));
        EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));

        communication.dataReceived(std::vector<uint8_t>{ 'e', 'f', 'g', '\r' });
    }
};

TEST_F(TerminalTest, echo_printable_characters)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { ' ' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'A' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'Z' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '~' } }), testing::_));
    
    communication.dataReceived(std::vector<uint8_t>{ ' ', 'A', 'Z', '~' });
    
    ExecuteAllActions();
}

TEST_F(TerminalTest, echo_bell_for_nonprintable_characters)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\a' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\a' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 31, 128 });

    ExecuteAllActions();
}

TEST_F(TerminalTest, ignore_newline_character)
{
    communication.dataReceived(std::vector<uint8_t>{ '\n' });

    ExecuteAllActions();
}

TEST_F(TerminalTest, echo_carriage_return_line_feed_and_prompt_on_carriage_return)
{

    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r', '\n' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ '\r' });

    ExecuteAllActions();
}

TEST_F(TerminalTest, ignore_unparsed_escaped_data)
{
    communication.dataReceived(std::vector<uint8_t>{ 27, ';', '[', 'O', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' });
    
    ExecuteAllActions();
}

TEST_F(TerminalTest, bell_on_invalid_escape_sequence)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\a' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 27, 'Z' });

    ExecuteAllActions();
}

TEST_F(TerminalTest, navigation_with_empty_buffer)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 1, 1, 5, 5 });

    ExecuteAllActions();
}

TEST_F(TerminalTest, clear_buffer_prints_prompt)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 3 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_home)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 1, 1 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_home_escape)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 27, 'H' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_end)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'a', 'b', 'c', 'd' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 1, 5, 5 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_end_escape)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'a', 'b', 'c', 'd' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 1, 27, 'F' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_left)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\b' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 2 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_left_escape)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\b' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 27, 'D' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, bell_on_move_past_beginning)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\b' } }), testing::_)).Times(4);
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\a' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 2, 2, 2, 2, 2 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_right)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\b' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'd' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 2, 6 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, move_cursor_right_escape)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\b' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'd' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 2, 27, 'C' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, bell_on_move_past_end)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\a' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 6 });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, remove_last_character_on_backspace)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\b' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { ' ', '\b' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ '\b' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, remove_first_character_on_backspace)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\b' } }), testing::_)).Times(4);
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'b', 'c', 'd' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { ' ', '\b' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\b' } }), testing::_)).Times(3);

    communication.dataReceived(std::vector<uint8_t>{ 2, 2, 2, '\b' });

    ExecuteAllActions();
}

TEST_F(TerminalNavigationTest, remove_character_with_delete)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\b' } }), testing::_)).Times(3);
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'c', 'd' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { ' ', '\b' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\b' } }), testing::_)).Times(2);

    communication.dataReceived(std::vector<uint8_t>{ 2, 2, 2, 4 });

    ExecuteAllActions();
}

TEST_F(TerminalTest, move_backward_in_history_with_empty_history)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 14 });

    ExecuteAllActions();
}

TEST_F(TerminalTest, move_forward_in_history_with_empty_history)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\a' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 16 });

    ExecuteAllActions();
}

TEST_F(TerminalHistoryTest, move_backward_in_history)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'e', 'f', 'g'} }), testing::_));

    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { 'a', 'b', 'c', 'd'} }), testing::_));

    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\a' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 16, 16, 16 });

    ExecuteAllActions();
}

TEST_F(TerminalHistoryTest, move_forward_past_end_prints_prompt)
{
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));

    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '\r' } }), testing::_));
    EXPECT_CALL(streamWriterMock, Insert(infra::CheckByteRangeContents(std::vector<uint8_t>{ { '>', ' ' } }), testing::_));

    communication.dataReceived(std::vector<uint8_t>{ 14, 14 });

    ExecuteAllActions();
}